#include <cmath>
#include <cstdlib>
#include <Eigen/Dense>

namespace ins {

typedef Eigen::Matrix3d matrix3d;
typedef Eigen::Vector3d vector3d;

using std::sin;
using std::cos;

/// stores absolute orientation, speed and location
struct state
{
	matrix3d orientation;
	vector3d speed;
	vector3d location;

	state();
};

state::state()
: orientation(matrix3d::Identity())
, speed(0,0,0)
, location(0,0,0)
{}

/// computes matrix L so that L*x = cross(lhs,x)
/// where cross(.) refers to the cross product.
matrix3d cross(vector3d const& lhs)
{
	matrix3d r; r <<
		  0    ,-lhs(2), lhs(1),
		 lhs(2),  0    ,-lhs(0),
		-lhs(1), lhs(0),  0;
	return r;
}

/// turns a 3D rotation angle into a rotation matrix
matrix3d rotation_vec_to_matrix(vector3d angles)
{
	// Rodrigues' formula
	matrix3d r = matrix3d::Identity();
	double len = angles.norm();
	if (len>0) {
		angles /= len; // normalized rotation axis
		matrix3d const t1 = cross(angles);
		matrix3d const t2 = t1*t1;
		r.noalias() += sin(len) * t1 + (1-cos(len)) * t2;
	}
	return r;
}

/// normalizes a row of a 3x3 matrix approximativly
void norm_mat_row(matrix3d & m, int idx)
{
	m.row(idx) *= 1.5 - 0.5 * m.row(idx).dot(m.row(idx));
}

void fix_orientation_rounding_errors(matrix3d & o)
{
	norm_mat_row(o,0);
	o.row(2) = o.row(0).cross(o.row(1));
	norm_mat_row(o,2);
	o.row(1) = o.row(2).cross(o.row(0));
}

/// update state using a rotation speed (in rad/s) and acceleration (in m/s^2)
void update(state & s, double timestep, vector3d const& rotspeed, vector3d const& acc)
{
	double thalf = 0.5*timestep;
	matrix3d const rhalf = rotation_vec_to_matrix(rotspeed*thalf);
	matrix3d const ohalf = s.orientation * rhalf;
	vector3d const shinc = thalf * ohalf * acc;
	vector3d const shalf = s.speed + shinc;
	s.location.noalias() += shalf * timestep;
	s.speed.noalias() = shalf + shinc;
	s.orientation.noalias() = ohalf * rhalf;
}

} // namespace ins

#include <iostream>
#include <string>
#include <sstream>
#include <stdint.h>
#include <vector>

namespace {

using namespace std;
using namespace ins;

void describe(state const& s)
{
	cout << "orientation:\n" << s.orientation <<
	"\nspeed\n" << s.speed <<
	"\nlocation\n" << s.location << endl;
}

void test01()
{
	state s;
	describe(s);
	for (int i=0; i<5; ++i) {
		update(s,0.1,vector3d(0,0,0),vector3d((i==0)-(i==3),0,0));
		describe(s);
	}
}

void test02()
{
	state s;
	describe(s);
	for (int i=0; i<5; ++i) {
		update(s,0.1,vector3d((i==0)-(i==3),0,0),vector3d(0,0,0));
		describe(s);
	}
}

struct readout
{
	uint32_t t0, t1;
	int16_t ax, ay, az;
	int16_t gx, gy, gz;
};

readout parse_readout(string const& line)
{
	readout ro;
	string dummy;
	stringstream ss (line);
	ss >> dummy >> ro.t0 >> dummy >> ro.t1;
	ss >> dummy >> ro.ax >> dummy >> ro.ay >> dummy >> ro.az;
	ss >> dummy >> ro.gx >> dummy >> ro.gy >> dummy >> ro.gz;
	return ro;
}

inline double sqr(double x)
{
	return x*x;
}

class conditional_moving_sum
{
public:
	conditional_moving_sum(int n_, long tolerance_)
		: tolerance(tolerance_), sum(0), n(n_), index(0)
	{}
	void feed(int value)
	{
		if (values.size()<n) {
			values.push_back(value);
			sum += value;
		} else {
			long avg = sum/n;
			if (abs(avg-value)<=tolerance) {
				int& store = values[index];
				int diff = value - store;
				store = value;
				sum += diff;
				if (++index == n) {
					index = 0;
				}
			}
		}
	}
	int average() const
	{
		if (values.empty()) return 0;
		if (values.size()==n) return (sum+(sum>0 ? n/2 : -n/2))/n;
		return int(sum*(values.size()+0.0)/n/n);
	}
private:
	vector<int> values;
	long tolerance;
	long sum;
	int n;
	int index;
};

void live()
{
	cerr << "Inertial Navigation Integrator" << endl;
	string line;
	const double g = 9.81;
	const double pi = atan(1.0)*4;
	const double acc_range = 2*g; // in m/s^2
	const double gyr_range = 250; // in degree/s
	const double acc_multiplier = acc_range/32768;
	const double gyr_multiplier = gyr_range*pi/180/32768;
	state s;
	unsigned ctr = 0;
	double time = 0;
	readout ro;
	bool invalid = true;
	conditional_moving_sum cms_gx (500,150);
	conditional_moving_sum cms_gy (500,150);
	conditional_moving_sum cms_gz (500,150);
	while (getline(cin,line)) {
		readout ro_old = ro;
		ro = parse_readout(line);
		if (invalid) {
			invalid = false;
		} else {
			double timestep = uint32_t(ro.t0-ro_old.t0)/20000000.0; // in seconds
			time += timestep;
			ro.gx +=  90; cms_gx.feed(ro.gx); ro.gx -= cms_gx.average();
			ro.gy += -30; cms_gy.feed(ro.gy); ro.gy -= cms_gy.average();
			ro.gz += 200; cms_gz.feed(ro.gz); ro.gz -= cms_gz.average();
			vector3d const axyz = vector3d(0,0,0)*acc_multiplier;
			vector3d const gxyz = vector3d(ro.gx,ro.gy,ro.gz)*gyr_multiplier;
			update(s,timestep,gxyz,axyz);
			if (ctr==0) {
				fix_orientation_rounding_errors(s.orientation);
				matrix3d const& o = s.orientation;
				double abs_degrees = sqrt(sqr(o(0,1))+sqr(o(0,2))+sqr(o(1,2)))*180/pi;
				cout << ro.t0 << ", " << ro.t1 << ", " << ", "
				     << ro.ax << ", " << ro.ay << ", " << ro.az << ", "
				     << ro.gx << ", " << ro.gy << ", " << ro.gz << "\n"
				     << s.orientation << "\n"
				     << time << " -- " << abs_degrees << "\n"
				     << "ori-avgs: " << cms_gx.average() << ", " << cms_gy.average() << ", " << cms_gz.average() << endl;
			}
			ctr = (ctr+1) & 0xF;
		}
	}
	test02();
}

} // anonymous namespace

int main()
{
	live();
	return 0;
}


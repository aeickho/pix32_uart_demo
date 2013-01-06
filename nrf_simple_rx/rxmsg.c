#include "rxmsg.h"



//Prework for tfec3d
#define WORDS_PER_FRAGMENT   6
#define BYTES_PER_FRAGMENT  (WORDS_PER_FRAGMENT*4)
#define MAX_DATA_FRAGMENTS  15
#define MAX_RECOV_FRAGMENTS  3
#define MAX_MESSAGE_SIZE    (BYTES_PER_FRAGMENT*MAX_DATA_FRAGMENTS)
#define FRAME_BUFF_SIZE     (MAX_DATA_FRAGMENTS+MAX_RECOV_FRAGMENTS)

#define NUM_DATA_FRAGS(pf)  ((pf)->metad >> 12)
#define FRAGMENT_INDEX(pf)  ((pf)->metad &  63)


struct frame new_frame;

struct frame
{
  tfec3_u32 mid;
  tfec3_u32 fragmentdata[WORDS_PER_FRAGMENT];
  unsigned short metad;
  unsigned short crc16;
};

struct sframe
{
  unsigned int timeout;
  unsigned int seqnr;
  struct frame frame;
};

struct mid_processeds
{
  tfec3_u32 mid;
  int cnt;
};

//FAMECACHE
#define FRAMECACHESIZE	15
struct sframe framecache[FRAMECACHESIZE];


tfec3_u32 *fragdatas[FRAME_BUFF_SIZE];


#define NEW_MESSAGE     0
#define MID_PROCESSED   1
#define MID_COMPLEATE   2
#define MID_ADDED	3

#define MID_PROCESSEDVAL	-1
#define MID_COMPLEATEVAL	-2
#define MID_PROCESSED_SIZE	5


struct mid_processeds mid_processed[MID_PROCESSED_SIZE];
int mid_processed_wp;


int
analyse_frame (void)
{

  int result = NEW_MESSAGE;
  int i;

    

  // mid_processed[i].cnt == -1 -> mid processed

  for (i = 0; i < MID_PROCESSED_SIZE; i++)
    {
      if (mid_processed[i].mid == new_frame.mid)
	{
	  if (mid_processed[i].cnt == MID_PROCESSEDVAL)
	    {
	      result = MID_PROCESSED;
	      break;
	    }
	  mid_processed[i].cnt++;
	  result = MID_ADDED;

	  if (mid_processed[i].cnt == NUM_DATA_FRAGS (&new_frame))
	    {
	      result = MID_COMPLEATE;
	      mid_processed[i].cnt = MID_COMPLEATEVAL;	    
	    }
	}
    }

  if (result == NEW_MESSAGE)
    {
      mid_processed[mid_processed_wp].mid = new_frame.mid;
      mid_processed[mid_processed_wp].cnt = 1;
      mid_processed_wp++;
      mid_processed_wp %= MID_PROCESSED_SIZE;
    }
  
  return (result);
}

void
add_to_framechache (void)
{
  int i;
  int seq_nr_id = 0;
  int seq_nr_diff_min;

  static int seq_nr;

  for (seq_nr_diff_min = 0, i = 0; i < FRAMECACHESIZE; i++)
    {
      if (seq_nr - framecache[i].seqnr > seq_nr_diff_min)
	{
	  seq_nr_diff_min = seq_nr - framecache[i].seqnr;
	  seq_nr_id = i;
	}
    }
  memcpy (&framecache[seq_nr_id].frame, &new_frame, 32);
  framecache[seq_nr_id].seqnr = seq_nr;
  seq_nr++;
}


int
process_framechache (uint32_t mid)
{
  unsigned char tx_valid[FRAME_BUFF_SIZE];
  struct frame spare_frame[3];

  int tx_valid_max = 0;
  char nr_data_frames = FRAME_BUFF_SIZE;
  int ok_data_frames = 0;

  int i, ii;

  memset (tx_valid, 0, sizeof (tx_valid));
  memset (fragdatas, 0, nr_data_frames + 3);

  for (i = 0; i < FRAMECACHESIZE; i++)
    {
      if (framecache[i].frame.mid == mid)
	{
	  tx_valid[FRAGMENT_INDEX (&framecache[i].frame)] = 1;
	  fragdatas[FRAGMENT_INDEX
		    (&framecache
		     [i].frame)] = framecache[i].frame.fragmentdata;
	  nr_data_frames = (NUM_DATA_FRAGS (&framecache[i].frame));
	  if ((FRAGMENT_INDEX (&framecache[i].frame)) > tx_valid_max)
	    tx_valid_max = FRAGMENT_INDEX (&framecache[i].frame);
	}
    }

  // generate spare space for redundance   
  for (ii = 0, i = 0; i < nr_data_frames + 3; i++)
    {
      if (i < nr_data_frames && tx_valid[i])
	ok_data_frames++;

      if (tx_valid[i] == 0)
	fragdatas[i] = spare_frame[ii++].fragmentdata;
    }
  tfec3_decode (WORDS_PER_FRAGMENT,
		nr_data_frames,
		tx_valid_max - nr_data_frames + 1, tx_valid, fragdatas);
  return (nr_data_frames);
}


void
rxmsg_process_frame (uint8_t * inData)
{
  int ret;

  memcpy (&new_frame, inData, 32);

  ret = analyse_frame ();
  if (ret == MID_PROCESSED)
    return;

  add_to_framechache ();

  if (ret != MID_COMPLEATE)
    return;

}




//mid_processed
//fragdatas[]


uint32_t get_mid_mid_processed(int i)
{
uint32_t mid;

mid=mid_processed[i].mid;

return (mid);
}

int  get_mid_cnt_processed(int i)
{
int cnt;

cnt=mid_processed[i].cnt;

return (cnt);
}


int
get_mid_prcessedindex (uint32_t mid)
{
  int i;
  
  uint32_t mid_temp;
  
   for (i = 0; i < MID_PROCESSED_SIZE; i++)
    {
      if (get_mid_mid_processed(i) == mid)
	{
	  return (i);
	}
    }
  return (-1);
}

int
get_mid_compleate (uint32_t * mid)
{
  int i;
  
  for (i = 0; i < MID_PROCESSED_SIZE; i++)
    {
      if (get_mid_cnt_processed(i) == MID_COMPLEATEVAL)
	{
	  *mid = get_mid_mid_processed(i);
	  return (0);
	}
    }
  return (-1);
}

rxmsg_get_frame (uint8_t * outData)
{
  uint32_t mid;
  int i;
  int nr_data_frames;
  int ret;


  ret = get_mid_compleate (&mid);

  if (ret == 0)
    {
      nr_data_frames = process_framechache (mid);
      mid_processed[get_mid_prcessedindex (mid)].cnt = MID_PROCESSEDVAL;

      for (i = 0; i < nr_data_frames; i++)
	memcpy (outData + (i * BYTES_PER_FRAGMENT), fragdatas[i],
		BYTES_PER_FRAGMENT);

      return (nr_data_frames * BYTES_PER_FRAGMENT);
    }
  else
    return (0);
}

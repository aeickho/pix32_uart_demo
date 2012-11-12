uint8_t nrf_read_reg (const uint8_t reg);
void nrf_write_reg (const uint8_t reg, const uint8_t val);
void nrf_write_long (const uint8_t cmd, int len, const uint8_t * data);

#define nrf_write_reg_long(reg, len, data) \
    nrf_write_long(C_W_REGISTER|(reg), len, data)


uint8_t nrf_cmd_status (uint8_t data);

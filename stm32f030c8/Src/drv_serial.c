/**
 ******************************************************************************
 * @file    drv_serial.c
 * @author  MEMS Application Team
 * @version V1.1
 * @date    10-August-2016
 * @brief   drv_serial source file
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "drv_serial.h"
#include "stdarg.h"
#include "string.h"
#include "drv.h"

/* Private variables ---------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern CRC_HandleTypeDef hcrc;
extern I2C_HandleTypeDef hi2c2;

static uint32_t tickstart;

#define ARR_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define MULTI_CMD_MAX 20

typedef int8_t (*hdlr_func)(uint8_t code, uint16_t param);
typedef struct {
  uint8_t cmd_mask;
  hdlr_func handler;
}cmd_table_t;

/* Private function prototypes -----------------------------------------------*/
static int8_t Drv_CMD_Handler(uint8_t *cmd);
static uint8_t Drv_SERIAL_Write_Act(uint8_t * pData);
static uint8_t Drv_SERIAL_Read_Act(uint8_t * pData);
static uint8_t Drv_SERIAL_Write_Rpt(uint8_t * pData);
static uint8_t Drv_SERIAL_Read_Rpt(uint8_t * pData);
static uint8_t Drv_SERIAL_Write(uint8_t * pData, uint32_t Timeout);
static uint8_t Drv_SERIAL_Read(uint8_t * pData, uint32_t Timeout);

int8_t Drv_THERM_CMD_Handler(uint8_t code, uint16_t param);
int8_t Drv_MOTOR_CMD_Handler(uint8_t code, uint16_t param);
int8_t Drv_IR_CMD_Handler(uint8_t code, uint16_t param);
int8_t Drv_HDMI_RCVR_CMD_Handler(uint8_t code, uint16_t param);
int8_t Drv_FAN_CMD_Handler(uint8_t code, uint16_t param);
int8_t Drv_EEPROM_CMD_Handler(uint8_t code, uint16_t param);
int8_t Drv_DLPC_CMD_Handler(uint8_t code, uint16_t param);
int8_t Drv_ACC_CMD_Handler(uint8_t code, uint16_t param);

//static void handle_power_key(void);
void handle_power_key(void);
static void handle_func_keys(uint16_t key);
static void handle_func_MIkeys(uint16_t key);

static cmd_table_t cmd_tbl[] = {
  {CMD_CODE_MASK_IR,    Drv_IR_CMD_Handler},
  {CMD_CODE_MASK_HDMI,  Drv_HDMI_RCVR_CMD_Handler},
  {CMD_CODE_MASK_DLPC,  Drv_DLPC_CMD_Handler},
  {CMD_CODE_MASK_ACC,   Drv_ACC_CMD_Handler},  
  //{CMD_CODE_MASK_AU,    Drv_AU_AMP_CMD_Handler},
  //{CMD_CODE_MASK_THERM, Drv_THERM_Handler},
  {CMD_CODE_MASK_MOTOR, Drv_MOTOR_CMD_Handler},
  {CMD_CODE_MASK_FAN,   Drv_FAN_CMD_Handler},
};

static uint8_t single_cmd[CMD_LEN_MAX];
static struct {
  uint8_t wr_id;
  uint8_t rd_id;
  uint8_t multi_cmd[MULTI_CMD_MAX][CMD_LEN_MAX];
} rpt_cmd, act_cmd;


/**
  * @brief  The I2c deinit.
  */
void MX_I2C_DeInit()
{
  HAL_I2C_DeInit(&hi2c2);
}


/**
  * @brief  The drv_serial init.
  */
void Drv_SERIAL_Init(void)
{
  memset((uint8_t*)&rpt_cmd, 0, sizeof(rpt_cmd));
  memset((uint8_t*)&act_cmd, 0, sizeof(act_cmd));
}

/**
  * @brief  The drv_serial init.
  */
void Drv_SERIAL_Proc(void)
{
  /* 1. read the uart buffer... */
  if (HAL_OK == Drv_SERIAL_Read(single_cmd, 0))
  {
    if (Drv_CMD_Handler(single_cmd) > 0)
    {
      if (CMD_HEADER_REQ == single_cmd[0])
      {
        single_cmd[0] = CMD_HEADER_RSP;
        Drv_SERIAL_Write(single_cmd, 0);
      }
    }
  }
  
  /* 2. read the action buffer... */
  if (HAL_OK == Drv_SERIAL_Read_Act(single_cmd))
  {
    //Drv_SERIAL_Log("Act location:0x%x 0x%x 0x%x 0x%x\r\n",
    //        single_cmd[0],single_cmd[1],single_cmd[2],single_cmd[3]);
    (void)Drv_CMD_Handler(single_cmd);
  }
#if 0
  /* 3. read the report buffer... */
  if (HAL_OK == Drv_SERIAL_Read_Rpt(single_cmd) &&
        single_cmd[2] != 0x01 && 
        single_cmd[2] != 0x07 && 
        single_cmd[2] != 0x09)
  {
    //Drv_SERIAL_Log("Tx Raspberry:0x%x 0x%x 0x%x 0x%x\r\n",
    //    single_cmd[0],single_cmd[1],single_cmd[2],single_cmd[3]);
    (void)Drv_SERIAL_Write(single_cmd, HAL_MAX_DELAY);
  }
#endif
}

void Drv_SERIAL_Log(const char *format, ...)
{
#if Raspberry
  return;
#else
#define MAX_LOG_LEN 64
  static char log[MAX_LOG_LEN];
  char log_len;
  va_list args;

  memset((uint8_t*)log, 0, MAX_LOG_LEN);
  va_start(args, format);  
  vsnprintf(log, MAX_LOG_LEN, format, args);
  va_end(args);

  log_len = strlen(log);
  if (log_len){
    log[log_len++] = '\n';
    HAL_UART_Transmit(&huart1, (uint8_t*)log, log_len, HAL_MAX_DELAY);
  }
#endif
}

uint8_t Drv_SERIAL_Act(uint8_t code, uint16_t param)
{
  uint8_t cmd[CMD_LEN_MAX];
  cmd[0] = CMD_HEADER_REQ;
  cmd[1] = code;
  cmd[2] = ((uint8_t*)&param)[0];
  cmd[3] = ((uint8_t*)&param)[1];
  return Drv_SERIAL_Write_Act(cmd);
}

uint8_t Drv_SERIAL_Rpt(uint8_t code, uint16_t param)
{
  uint8_t cmd[CMD_LEN_MAX];
  cmd[0] = CMD_HEADER_RPT;
  cmd[1] = code;
  cmd[2] = ((uint8_t*)&param)[0];
  cmd[3] = ((uint8_t*)&param)[1];
  
  return Drv_SERIAL_Write_Rpt(cmd);
}

static int8_t Drv_CMD_Handler(uint8_t *cmd)
{  
  if(CMD_HEADER_REQ == cmd[0] || CMD_HEADER_REQ_NR == cmd[0])
  {
    uint8_t i;
    uint16_t cmdCode;

    cmdCode = cmd[2] | (cmd[3]<<8);
    for (i = 0; i < ARR_SIZE(cmd_tbl); i ++)
    {
      if (GET_MASK(cmd[1]) == cmd_tbl[i].cmd_mask)
      {
        if (cmd_tbl[i].handler)
        {
          return cmd_tbl[i].handler(GET_OP(cmd[1]), cmdCode);
        }
      }
    }
  }
  return -1;
}

static uint8_t Drv_SERIAL_Write_Rpt(uint8_t * pData)
{
  memcpy(rpt_cmd.multi_cmd[rpt_cmd.wr_id], pData, CMD_LEN_MAX-1);
  rpt_cmd.wr_id = ( rpt_cmd.wr_id + 1 ) % MULTI_CMD_MAX;
  
  return (uint8_t)HAL_OK;
}

static uint8_t Drv_SERIAL_Read_Rpt(uint8_t * pData)
{
  if (rpt_cmd.rd_id != rpt_cmd.wr_id)
  {
    memcpy(pData, rpt_cmd.multi_cmd[rpt_cmd.rd_id], CMD_LEN_MAX-1);
    rpt_cmd.rd_id = ( rpt_cmd.rd_id + 1 ) % MULTI_CMD_MAX;
    return (uint8_t)HAL_OK;
  }
  return (uint8_t)HAL_ERROR;
}

static uint8_t Drv_SERIAL_Write_Act(uint8_t * pData)
{
  memcpy(act_cmd.multi_cmd[act_cmd.wr_id], pData, CMD_LEN_MAX-1);
  act_cmd.wr_id = ( act_cmd.wr_id + 1 ) % MULTI_CMD_MAX;
  
  return (uint8_t)HAL_OK;
}

static uint8_t Drv_SERIAL_Read_Act(uint8_t * pData)
{
  if (act_cmd.rd_id != act_cmd.wr_id)
  {
    memcpy(pData, act_cmd.multi_cmd[act_cmd.rd_id], CMD_LEN_MAX-1);
    act_cmd.rd_id = ( act_cmd.rd_id + 1 ) % MULTI_CMD_MAX;    
    return (uint8_t)HAL_OK;
  }  
  return (uint8_t)HAL_ERROR;
}

static uint8_t Drv_SERIAL_Write(uint8_t * pData, uint32_t Timeout)
{
  pData[CMD_LEN_MAX-1] = (uint8_t)HAL_CRC_Calculate(&hcrc, (uint32_t*)pData, CMD_LEN_MAX-1);
  return (uint8_t)HAL_UART_Transmit(&huart1, pData, CMD_LEN_MAX, Timeout);
}

static uint8_t Drv_SERIAL_Read(uint8_t * pData, uint32_t Timeout)
{
  uint8_t re;
  re = (uint8_t)HAL_UART_Receive(&huart1, pData, CMD_LEN_MAX, Timeout);
  if (re == (uint8_t)HAL_OK)
  {
    uint8_t crc;
    crc = (uint8_t)HAL_CRC_Calculate(&hcrc, (uint32_t*)pData, CMD_LEN_MAX-1);
    if (crc == pData[CMD_LEN_MAX-1])
    {
      re = (uint8_t)HAL_OK;
    }
    else
    {
      re = (uint8_t)HAL_ERROR;
    }
  }
  return re;
}

/* ----------------------------------------------------------- */

static int8_t Drv_THERM_CMD_Handler(uint8_t code, uint16_t param)
{
  int8_t rc = 0;
    
  switch (code)
  {
  case CMD_OP_THERM_GET_VALUE:
    rc = drv_therm_get_value();
    break;
  }
  
  return rc;
}

static int8_t Drv_MOTOR_CMD_Handler(uint8_t code, uint16_t param)
{
  int8_t rc = HAL_OK;
    
  switch (code)
  {
  case CMD_OP_MOTOR_SET_FORWARD:
    drv_motor_move_forward(4);
    break;
  case CMD_OP_MOTOR_SET_BACKWARD:
    drv_motor_move_reverse(4);
    break;
  }
  
  return rc;
}

int8_t Drv_IR_CMD_Handler(uint8_t code, uint16_t key)
{
  //Drv_SERIAL_Log("Drv_IR_CMD_Handler 0x%x\r\n",key);
  static uint8_t  flag = 0;
  static uint16_t lastKey = 0;

  if (code == CMD_OP_IR_CODE)
  {

    if(key>>8 == 0xff) {// MI controler

      if(lastKey != key) {

        lastKey = key;
        flag = 1;
        tickstart = HAL_GetTick();
      } else {

        if((HAL_GetTick() - tickstart) > ((REMOTE_MI_POWER == key) ? 2000 : 300)) //repeat press
        {
          tickstart = HAL_GetTick();
          flag = 1;
        } else {
            //Drv_SERIAL_Log("repeat escap\r\n");
        }
      }

      if(flag) {
        //Drv_SERIAL_Log("TX key:%x\r\n",key);
        flag = 0;
        switch (key)
        {
          case REMOTE_MI_POWER:
            handle_power_key();
            break;
          default:
            handle_func_MIkeys(key);
        }
      }
    } else {//NEC controler
    
      switch (key>>8)
      {
        case REMOTE_NEC_POWER:
          handle_power_key();
          break;
        default:
          handle_func_keys(key);
      }
    }
  }
  return 0;
}

int8_t Drv_HDMI_RCVR_CMD_Handler(uint8_t code, uint16_t param)
{
  int8_t rc = -1;

  switch (code)
  {
  case CMD_OP_HDMI_GET_STATUS:
    drv_hdmi_get_p0_status();
    rc = HAL_OK;
    break;
  }
  
  return rc;
}

int8_t Drv_FAN_CMD_Handler(uint8_t code, uint16_t param)
{
  int8_t rc = 0;

  switch (code)
  {
  case CMD_OP_FAN_SPEED:
    drv_fan_speed(param);
    break;
  case CMD_OP_FAN_ON:
    drv_fan_on();
    break;
  case CMD_OP_FAN_OFF:
    drv_fan_off(0);
    break;
  }

  return rc;
}

int8_t Drv_EEPROM_CMD_Handler(uint8_t code, uint16_t param)
{
  int8_t rc = -1;

  switch (code)
  {
  case CMD_OP_EEPROM_READ_EDID:
    drv_eeprom_read_edid();
    rc = HAL_OK;
    break;
  case CMD_OP_EEPROM_WRITE_EDID:
    drv_eeprom_write_edid();
    rc = HAL_OK;
    break;
  }
  
  return rc;
}

int8_t Drv_DLPC_CMD_Handler(uint8_t code, uint16_t param)
{
  int8_t rc = 0;

  switch (code)
  {
    case CMD_OP_DLPC_SET_PROJ_ON:
      rc = drv_dlpc_proj_ctrl(param);
      break;
    case CMD_OP_DLPC_SET_CURRENT:
      rc = drv_dlpc_set_current(0, param);
      break;
    case CMD_OP_DLPC_SET_INPUT:
      rc = drv_dlpc_set_input(param);
      break;
    case CMD_OP_DLPC_SET_ORIENT:
      rc = drv_dlpc_set_orient();
      break;
    case CMD_OP_DLPC_SET_OUPUT_CTRL:
      break;
    case CMD_OP_DLPC_SET_KEYSTONE_CTRL:
      rc = drv_dlpc_reset_keystone();
      break;
    case CMD_OP_DLPC_SET_KEYSTONE_ANGLE:
      rc = drv_dlpc_set_keystone(param);
      break;
    case CMD_OP_DLPC_DEBUG_SET_LED_CURRENT_UP:
      break;
    case CMD_OP_DLPC_DEBUG_SET_LED_CURRENT_DN:
      break;
    case CMD_OP_DLPC_DEBUG_SET_LED_CURRENT_SAVE:
      break;
    case CMD_OP_DLPC_DEBUG_GET_SHORT_STATUS:
      break;
    case CMD_OP_DLPC_DEBUG_SWITCH_TEST_PATTERN:
      rc = drv_dlpc_switch_test_pattern();
      break;    
    case CMD_OP_DLPC_DEBUG_SW:
      rc = drv_dlpc_sw();
  }
  
  return rc;
}

int8_t Drv_ACC_CMD_Handler(uint8_t code, uint16_t param)
{
  return -1;
}

/* ---------------------------------------------------- */
static uint32_t tickstart;
//static void handle_power_key(void) 
void handle_power_key(void)
{
  drv_dlpc_proj_ctrl(2);
  if (DLPC_PROJ_STATUS())
  {
    drv_fan_on();
    HAL_Delay(500);
    MX_I2C2_Init();
    drv_dlpc_set_input(2);
    // pull up the HPB signal
    drv_hdmi_set_hpd(1);
    drv_hdmi_set_output(1);
  }
  else
  {
    // pull down the HPB signal
    drv_hdmi_set_hpd(0);
    drv_hdmi_set_output(0);
    MX_I2C_DeInit();
    drv_fan_off(5000);
  }
}
static void handle_func_keys(uint16_t key)
{
  switch (key>>8)
{
    case KEY_CHUP:
      Drv_MOTOR_CMD_Handler(CMD_OP_MOTOR_SET_FORWARD, 4);
      break;
    case KEY_CHDN:
      Drv_MOTOR_CMD_Handler(CMD_OP_MOTOR_SET_BACKWARD, 4);
      break;
    case KEY_1:
      Drv_DLPC_CMD_Handler(CMD_OP_DLPC_SET_PROJ_ON, 2);
      break;
    case KEY_2:
      Drv_DLPC_CMD_Handler(CMD_OP_DLPC_SET_INPUT, 2);
      break;
    case KEY_PREV:
      Drv_DLPC_CMD_Handler(CMD_OP_DLPC_SET_KEYSTONE_ANGLE, 0);
      break;
    case KEY_NEXT:
      Drv_DLPC_CMD_Handler(CMD_OP_DLPC_SET_KEYSTONE_ANGLE, 1);
      break;
    case KEY_PLAY:
      Drv_DLPC_CMD_Handler(CMD_OP_DLPC_SET_KEYSTONE_CTRL, 0);
      break;
    case KEY_VOLDN:
      Drv_DLPC_CMD_Handler(CMD_OP_DLPC_DEBUG_SWITCH_TEST_PATTERN, 0);
      break;
    case KEY_EQ:
      Drv_DLPC_CMD_Handler(CMD_OP_DLPC_SET_ORIENT, 0);
      break;
    case KEY_0:
      Drv_EEPROM_CMD_Handler(CMD_OP_EEPROM_READ_EDID, 0);
      break;
    case KEY_100:
      Drv_EEPROM_CMD_Handler(CMD_OP_EEPROM_WRITE_EDID, 0);
      break;
    case KEY_3:
      Drv_HDMI_RCVR_CMD_Handler(CMD_OP_HDMI_GET_STATUS, 0);
      break;
    case KEY_4:
      Drv_THERM_CMD_Handler(CMD_OP_THERM_GET_VALUE, 0);
      break;
    case KEY_5:
      Drv_DLPC_CMD_Handler(CMD_OP_DLPC_SET_CURRENT, 0);
      break;
    case KEY_6:
      Drv_DLPC_CMD_Handler(CMD_OP_DLPC_SET_CURRENT, 1);
      break;
    case KEY_7:
      Drv_DLPC_CMD_Handler(CMD_OP_DLPC_DEBUG_SW, 0);
      break;
    case KEY_9:
      HAL_NVIC_SystemReset();
  }
}

static void handle_func_MIkeys(uint16_t key)
{
  switch (key)
  {
    case REMOTE_MI_HOME:
        drv_dlpc_set_keystone(0);
        break;
    case REMOTE_MI_MENU:
        drv_dlpc_set_keystone(1);
        break;
    case REMOTE_MI_POWER:
    case REMOTE_MI_UP:
    case REMOTE_MI_DOWN:
    case REMOTE_MI_LEFT:
    case REMOTE_MI_RIGHT:
    case REMOTE_MI_OK:
    case REMOTE_MI_BACK:
    case REMOTE_MI_PLUS:
    case REMOTE_MI_MINUS:
#if 1
      memset(single_cmd, 0, sizeof(single_cmd));
      single_cmd[0] = CMD_HEADER_RPT;
      single_cmd[1] = SET_CODE(CMD_CODE_MASK_IR, CMD_OP_IR_CODE);
      single_cmd[2] = 0xff&key;
      single_cmd[3] = 0xff&(key>>8);
      Drv_SERIAL_Write(single_cmd, HAL_MAX_DELAY);
#endif
      break;
    default:
      Drv_SERIAL_Log("invalid key[0x%x]!\r\n",key);
  }
}



#include "stm32f10x.h"
#include "OLED_Font.h"
#include "OLED.h"
#include "Font.h"
#include "string.h"
/*引脚配置*/
#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x)) //PA5
#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))

//设OLED屏幕纵轴方向为y轴，�?轴为x轴，x表示�?几列 0-128 ，y表示�?几页 0-8

// OLED器件地址
#define OLED_ADDRESS 0x7A

// OLED参数
#define OLED_PAGE 8            // OLED页数
#define OLED_ROW 8 * OLED_PAGE // OLED行数
#define OLED_COLUMN 128        // OLED列数

// 显存
uint8_t OLED_GRAM[OLED_PAGE][OLED_COLUMN];

/*引脚初�?�化*/
void OLED_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
  OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C开�?
  * @param  �?
  * @retval �?
  */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}
/**
  * @brief  I2C停�??
  * @param  �?
  * @retval �?
  */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}
/**
  * @brief  I2C发送一�?字节
  * @param  Byte 要发送的一�?字节
  * @retval �?
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);	//额�?�的一�?时钟，不处理应答信号
	OLED_W_SCL(0);
}
/**
  * @brief  OLED写命�?
  * @param  Command 要写入的命令
  * @retval �?
  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命�?
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}

/**
  * @brief  OLED写数�?
  * @param  Data 要写入的数据
  * @retval �?
  */
void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数�?
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}
/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上�?�为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上�?�为原点，向右方向的坐标，范围：0~127
  * @retval �?
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置�?4�?
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置�?4�?
}
// ========================== 底层通信函数 ==========================

/**
 * @brief 向OLED发送数�?的函�?
 * @param data 要发送的数据
 * @param len 要发送的数据长度
 * @return None
 * @note 此函数是移�?�本驱动时的重�?�函�? 将本驱动库移植到其他平台时应根据实际情况�?改�?�函�?
 */
void OLED_Send(uint8_t *data, uint8_t len) {
	u8 i;
	for(;i<len;i++)
	{
		OLED_WriteData(data[i]);
	}
}

// ========================== OLED驱动函数 ==========================
/**
 * @brief 开启OLED显示
 */
void OLED_DisPlay_On() {
  OLED_WriteCommand(0x8D); // 电荷泵使�?
  OLED_WriteCommand(0x14); // 开�?电荷�?
  OLED_WriteCommand(0xAF); // 点亮屏幕
}

/**
 * @brief 关闭OLED显示
 */
void OLED_DisPlay_Off() {
  OLED_WriteCommand(0x8D); // 电荷泵使�?
  OLED_WriteCommand(0x10); // 关闭电荷�?
  OLED_WriteCommand(0xAE); // 关闭屏幕
}

/**
 * @brief 设置颜色模式 黑底白字或白底黑�?
 * @param ColorMode 颜色模式COLOR_NORMAL/COLOR_REVERSED
 * @note 此函数直接�?�置屏幕的�?�色模式
 */
void OLED_SetColorMode(OLED_ColorMode mode) {
  if (mode == OLED_COLOR_NORMAL) {
    OLED_WriteCommand(0xA6); // 正常显示
  }
  if (mode == OLED_COLOR_REVERSED) {
    OLED_WriteCommand(0xA7); // 反色显示
  }
}
// ========================== 显存操作函数 ==========================

/**
 * @brief 清空显存 绘制新的一�?
 */
void OLED_NewFrame() {
  memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
}

/**
 * @brief 将当前显存显示到屏幕�?
 * @note 此函数是移�?�本驱动时的重�?�函�? 将本驱动库移植到其他驱动�?片时应根�?实际情况�?改�?�函�?
 */
void OLED_ShowFrame() {
  static uint8_t sendBuffer[OLED_COLUMN + 1];
  sendBuffer[0] = 0x40;
  for (uint8_t i = 0; i < OLED_PAGE; i++) {
    OLED_WriteCommand(0xB0 + i); // 设置页地址
    OLED_WriteCommand(0x02);     // 设置列地址�?4�?
    OLED_WriteCommand(0x10);     // 设置列地址�?4�?
    memcpy(sendBuffer + 1, OLED_GRAM[i], OLED_COLUMN);
    OLED_Send(sendBuffer, OLED_COLUMN + 1);
  }
}

/**
 * @brief 设置一�?像素�?
 * @param x �?坐标
 * @param y 纵坐�?
 * @param color 颜色
 */
void OLED_SetPixel(uint8_t x, uint8_t y, OLED_ColorMode color) {
  if (x >= OLED_COLUMN || y >= OLED_ROW) return;
  if (!color) {
    OLED_GRAM[y / 8][x] |= 1 << (y % 8);
  } else {
    OLED_GRAM[y / 8][x] &= ~(1 << (y % 8));
  }
}

/**
 * @brief 设置显存�?一字节数据的某几位
 * @param page 页地址
 * @param column 列地址
 * @param data 数据
 * @param start 起�?�位
 * @param end 结束�?
 * @param color 颜色
 * @note 此函数将显存�?的某一字节的�?�start位到第end位�?�置为与data相同
 * @note start和end的范围为0-7, start必须小于等于end
 * @note 此函数与OLED_SetByte_Fine的区�?在于此函数只能�?�置显存�?的某一真实字节
 */
void OLED_SetByte_Fine(uint8_t page, uint8_t column, uint8_t data, uint8_t start, uint8_t end, OLED_ColorMode color) {
  static uint8_t temp;
  if (page >= OLED_PAGE || column >= OLED_COLUMN) return;
  if (color) data = ~data;

  temp = data | (0xff << (end + 1)) | (0xff >> (8 - start));
  OLED_GRAM[page][column] &= temp;
  temp = data & ~(0xff << (end + 1)) & ~(0xff >> (8 - start));
  OLED_GRAM[page][column] |= temp;
  // 使用OLED_SetPixel实现
  // for (uint8_t i = start; i <= end; i++) {
  //   OLED_SetPixel(column, page * 8 + i, !((data >> i) & 0x01));
  // }
}

/**
 * @brief 设置显存�?的一字节数据
 * @param page 页地址
 * @param column 列地址
 * @param data 数据
 * @param color 颜色
 * @note 此函数将显存�?的某一字节设置为data的�?
 */
void OLED_SetByte(uint8_t page, uint8_t column, uint8_t data, OLED_ColorMode color) {
  if (page >= OLED_PAGE || column >= OLED_COLUMN) return;
  if (color) data = ~data;
  OLED_GRAM[page][column] = data;
}

/**
 * @brief 设置显存�?的一字节数据的某几位
 * @param x �?坐标
 * @param y 纵坐�?
 * @param data 数据
 * @param len 位数
 * @param color 颜色
 * @note 此函数将显存�?�?(x,y)开始向下数len位�?�置为与data相同
 * @note len的范围为1-8
 * @note 此函数与OLED_SetByte_Fine的区�?在于此函数的�?坐标和纵坐标�?以像素为单位�?, �?能出现跨两个真实字节的情�?(跨页)
 */
void OLED_SetBits_Fine(uint8_t x, uint8_t y, uint8_t data, uint8_t len, OLED_ColorMode color) {
  uint8_t page = y / 8;
  uint8_t bit = y % 8;
  if (bit + len > 8) {
    OLED_SetByte_Fine(page, x, data << bit, bit, 7, color);
    OLED_SetByte_Fine(page + 1, x, data >> (8 - bit), 0, len + bit - 1 - 8, color);
  } else {
    OLED_SetByte_Fine(page, x, data << bit, bit, bit + len - 1, color);
  }
  // 使用OLED_SetPixel实现
  // for (uint8_t i = 0; i < len; i++) {
  //   OLED_SetPixel(x, y + i, !((data >> i) & 0x01));
  // }
}

/**
 * @brief 设置显存�?一字节长度的数�?
 * @param x �?坐标
 * @param y 纵坐�?
 * @param data 数据
 * @param color 颜色
 * @note 此函数将显存�?�?(x,y)开始向下数8位�?�置为与data相同
 * @note 此函数与OLED_SetByte的区�?在于此函数的�?坐标和纵坐标�?以像素为单位�?, �?能出现跨两个真实字节的情�?(跨页)
 */
void OLED_SetBits(uint8_t x, uint8_t y, uint8_t data, OLED_ColorMode color) {
  uint8_t page = y / 8;
  uint8_t bit = y % 8;
  OLED_SetByte_Fine(page, x, data << bit, bit, 7, color);
  if (bit) {
    OLED_SetByte_Fine(page + 1, x, data >> (8 - bit), 0, bit - 1, color);
  }
}

/**
 * @brief 设置一块显存区�?
 * @param x 起�?�横坐标
 * @param y 起�?�纵坐标
 * @param data 数据的起始地址
 * @param w 宽度
 * @param h 高度
 * @param color 颜色
 * @note 此函数将显存�?�?(x,y)开始的w*h�?像素设置为data�?的数�?
 * @note data的数�?应�?�采用列行式排列
 */
void OLED_SetBlock(uint8_t x, uint8_t y, const uint8_t *data, uint8_t w, uint8_t h, OLED_ColorMode color) {
  uint8_t fullRow = h / 8; // 完整的�?�数
  uint8_t partBit = h % 8; // 不完整的字节�?的有效位�?
  for (uint8_t i = 0; i < w; i++) {
    for (uint8_t j = 0; j < fullRow; j++) {
      OLED_SetBits(x + i, y + j * 8, data[i + j * w], color);
    }
  }
  if (partBit) {
    uint16_t fullNum = w * fullRow; // 完整的字节数
    for (uint8_t i = 0; i < w; i++) {
      OLED_SetBits_Fine(x + i, y + (fullRow * 8), data[fullNum + i], partBit, color);
    }
  }
  // 使用OLED_SetPixel实现
  // for (uint8_t i = 0; i < w; i++) {
  //   for (uint8_t j = 0; j < h; j++) {
  //     for (uint8_t k = 0; k < 8; k++) {
  //       if (j * 8 + k >= h) break; // 防�?�越�?(不完整的字节
  //       OLED_SetPixel(x + i, y + j * 8 + k, !((data[i + j * w] >> k) & 0x01));
  //     }
  //   }
  // }
}

// ========================== 图形绘制函数 ==========================
/**
 * @brief 绘制一条线�?
 * @param x1 起�?�点�?坐标
 * @param y1 起�?�点纵坐�?
 * @param x2 终�?�点�?坐标
 * @param y2 终�?�点纵坐�?
 * @param color 颜色
 * @note 此函数使用Bresenham算法绘制线�??
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color) {
  static uint8_t temp = 0;
  if (x1 == x2) {
    if (y1 > y2) {
      temp = y1;
      y1 = y2;
      y2 = temp;
    }
    for (uint8_t y = y1; y <= y2; y++) {
      OLED_SetPixel(x1, y, color);
    }
  } else if (y1 == y2) {
    if (x1 > x2) {
      temp = x1;
      x1 = x2;
      x2 = temp;
    }
    for (uint8_t x = x1; x <= x2; x++) {
      OLED_SetPixel(x, y1, color);
    }
  } else {
    // Bresenham直线算法
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;
    int16_t ux = ((dx > 0) << 1) - 1;
    int16_t uy = ((dy > 0) << 1) - 1;
    int16_t x = x1, y = y1, eps = 0;
	if(dx<0) dx=-dx;
	else dx=dx;
	if(dy<0) dy=-dy;
	else dy=dy;
//    dx = abs(dx);
//    dy = abs(dy);
    if (dx > dy) {
      for (x = x1; x != x2; x += ux) {
        OLED_SetPixel(x, y, color);
        eps += dy;
        if ((eps << 1) >= dx) {
          y += uy;
          eps -= dx;
        }
      }
    } else {
      for (y = y1; y != y2; y += uy) {
        OLED_SetPixel(x, y, color);
        eps += dx;
        if ((eps << 1) >= dy) {
          x += ux;
          eps -= dy;
        }
      }
    }
  }
}
/**
 * @brief 绘制一�?矩形
 * @param x 起�?�点�?坐标
 * @param y 起�?�点纵坐�?
 * @param w 矩形宽度
 * @param h 矩形高度
 * @param color 颜色
 */
void OLED_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color) {
  OLED_DrawLine(x, y, x + w, y, color);
  OLED_DrawLine(x, y + h, x + w, y + h, color);
  OLED_DrawLine(x, y, x, y + h, color);
  OLED_DrawLine(x + w, y, x + w, y + h, color);
}

/**
 * @brief 绘制一�?�?充矩�?
 * @param x 起�?�点�?坐标
 * @param y 起�?�点纵坐�?
 * @param w 矩形宽度
 * @param h 矩形高度
 * @param color 颜色
 */
void OLED_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color) {
  for (uint8_t i = 0; i < h; i++) {
    OLED_DrawLine(x, y+i, x+w-1, y+i, color);
  }
}

/**
 * @brief 绘制一�?三�?�形
 * @param x1 �?一�?点横坐标
 * @param y1 �?一�?点纵坐标
 * @param x2 �?二个点横坐标
 * @param y2 �?二个点纵坐标
 * @param x3 �?三个点横坐标
 * @param y3 �?三个点纵坐标
 * @param color 颜色
 */
void OLED_DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color) {
  OLED_DrawLine(x1, y1, x2, y2, color);
  OLED_DrawLine(x2, y2, x3, y3, color);
  OLED_DrawLine(x3, y3, x1, y1, color);
}

/**
 * @brief 绘制一�?�?充三角形
 * @param x1 �?一�?点横坐标
 * @param y1 �?一�?点纵坐标
 * @param x2 �?二个点横坐标
 * @param y2 �?二个点纵坐标
 * @param x3 �?三个点横坐标
 * @param y3 �?三个点纵坐标
 * @param color 颜色
 */
void OLED_DrawFilledTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color) {
  uint8_t a = 0, b = 0, y = 0, last = 0;
  if (y1 > y2) {
    a = y2;
    b = y1;
  } else {
    a = y1;
    b = y2;
  }
  y = a;
  for (; y <= b; y++) {
    if (y <= y3) {
      OLED_DrawLine(x1 + (y - y1) * (x2 - x1) / (y2 - y1), y, x1 + (y - y1) * (x3 - x1) / (y3 - y1), y, color);
    } else {
      last = y - 1;
      break;
    }
  }
  for (; y <= b; y++) {
    OLED_DrawLine(x2 + (y - y2) * (x3 - x2) / (y3 - y2), y, x1 + (y - last) * (x3 - x1) / (y3 - last), y, color);
  }
}

/**
 * @brief 绘制一�?�?
 * @param x 圆心�?坐标
 * @param y 圆心纵坐�?
 * @param r 圆半�?
 * @param color 颜色
 * @note 此函数使用Bresenham算法绘制�?
 */
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color) {
  int16_t a = 0, b = r, di = 3 - (r << 1);
  while (a <= b) {
    OLED_SetPixel(x - b, y - a, color);
    OLED_SetPixel(x + b, y - a, color);
    OLED_SetPixel(x - a, y + b, color);
    OLED_SetPixel(x - b, y - a, color);
    OLED_SetPixel(x - a, y - b, color);
    OLED_SetPixel(x + b, y + a, color);
    OLED_SetPixel(x + a, y - b, color);
    OLED_SetPixel(x + a, y + b, color);
    OLED_SetPixel(x - b, y + a, color);
    a++;
    if (di < 0) {
      di += 4 * a + 6;
    } else {
      di += 10 + 4 * (a - b);
      b--;
    }
    OLED_SetPixel(x + a, y + b, color);
  }
}

/**
 * @brief 绘制一�?�?充圆
 * @param x 圆心�?坐标
 * @param y 圆心纵坐�?
 * @param r 圆半�?
 * @param color 颜色
 * @note 此函数使用Bresenham算法绘制�?
 */
void OLED_DrawFilledCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color) {
  int16_t a = 0, b = r, di = 3 - (r << 1);
  while (a <= b) {
    for (int16_t i = x - b; i <= x + b; i++) {
      OLED_SetPixel(i, y + a, color);
      OLED_SetPixel(i, y - a, color);
    }
    for (int16_t i = x - a; i <= x + a; i++) {
      OLED_SetPixel(i, y + b, color);
      OLED_SetPixel(i, y - b, color);
    }
    a++;
    if (di < 0) {
      di += 4 * a + 6;
    } else {
      di += 10 + 4 * (a - b);
      b--;
    }
  }
}

/**
 * @brief 绘制一�?�?�?
 * @param x �?圆中心横坐标
 * @param y �?圆中心纵坐标
 * @param a �?圆长�?
 * @param b �?圆短�?
 */
void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, OLED_ColorMode color) {
  int xpos = 0, ypos = b;
  int a2 = a * a, b2 = b * b;
  int d = b2 + a2 * (0.25 - b);
  while (a2 * ypos > b2 * xpos) {
    OLED_SetPixel(x + xpos, y + ypos, color);
    OLED_SetPixel(x - xpos, y + ypos, color);
    OLED_SetPixel(x + xpos, y - ypos, color);
    OLED_SetPixel(x - xpos, y - ypos, color);
    if (d < 0) {
      d = d + b2 * ((xpos << 1) + 3);
      xpos += 1;
    } else {
      d = d + b2 * ((xpos << 1) + 3) + a2 * (-(ypos << 1) + 2);
      xpos += 1, ypos -= 1;
    }
  }
  d = b2 * (xpos + 0.5) * (xpos + 0.5) + a2 * (ypos - 1) * (ypos - 1) - a2 * b2;
  while (ypos > 0) {
    OLED_SetPixel(x + xpos, y + ypos, color);
    OLED_SetPixel(x - xpos, y + ypos, color);
    OLED_SetPixel(x + xpos, y - ypos, color);
    OLED_SetPixel(x - xpos, y - ypos, color);
    if (d < 0) {
      d = d + b2 * ((xpos << 1) + 2) + a2 * (-(ypos << 1) + 3);
      xpos += 1, ypos -= 1;
    } else {
      d = d + a2 * (-(ypos << 1) + 3);
      ypos -= 1;
    }
  }
}

/**
 * @brief 绘制一张图�?
 * @param x 起�?�点�?坐标
 * @param y 起�?�点纵坐�?
 * @param img 图片
 * @param color 颜色
 */
void OLED_DrawImage(uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color) {
  OLED_SetBlock(x, y, img->data, img->w, img->h, color);
}

// ================================ 文字绘制 ================================
void OLED_PrintASCIINum(uint8_t x, uint8_t y, uint32_t Number,uint8_t Length, const ASCIIFont *font, OLED_ColorMode color)
{
	u8 i;
	for(int i=0; i<Length;i++)
	{
		OLED_PrintASCIIChar(x, y + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0',font,color);
	}
}
/**
 * @brief 绘制一个ASCII字�??
 * @param x 起�?�点�?坐标
 * @param y 起�?�点纵坐�?
 * @param ch 字�??
 * @param font 字体
 * @param color 颜色
 */
void OLED_PrintASCIIChar(uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color) {
  OLED_SetBlock(x, y, font->chars + (ch - ' ') * (((font->h + 7) / 8) * font->w), font->w, font->h, color);
}

/**
 * @brief 绘制一个ASCII字�?�串
 * @param x 起�?�点�?坐标
 * @param y 起�?�点纵坐�?
 * @param str 字�?�串
 * @param font 字体
 * @param color 颜色
 */
void OLED_PrintASCIIString(uint8_t x, uint8_t y, char *str, const ASCIIFont *font, OLED_ColorMode color) {
  uint8_t x0 = x;
  while (*str) {
    OLED_PrintASCIIChar(x0, y, *str, font, color);
    x0 += font->w;
    str++;
  }
}

/**
 * @brief 获取UTF-8编码的字符长�?
 */
uint8_t _OLED_GetUTF8Len(char *string) {
  if ((string[0] & 0x80) == 0x00) {
    return 1;
  } else if ((string[0] & 0xE0) == 0xC0) {
    return 2;
  } else if ((string[0] & 0xF0) == 0xE0) {
    return 3;
  } else if ((string[0] & 0xF8) == 0xF0) {
    return 4;
  }
  return 0;
}

/**
 * @brief 绘制字�?�串
 * @param x 起�?�点�?坐标
 * @param y 起�?�点纵坐�?
 * @param str 字�?�串
 * @param font 字体
 * @param color 颜色
 *
 * @note 为保证字符串�?的中文会�?�?动识�?并绘�?, 需:
 * 1. 编译器字符集设置为UTF-8
 * 2. 使用波特律动LED取模工具生成字模(https://led.baud-dance.com)
 */
/**
 * @brief 绘制字�?�串
 * @param x 起�?�点�?坐标
 * @param y 起�?�点纵坐�?
 * @param str 字�?�串
 * @param font 字体
 * @param color 颜色
 *
 * @note 为保证字符串�?的中文会�?�?动识�?并绘�?, 需:
 * 1. 编译器字符集设置为UTF-8
 * 2. 使用波特律动LED取模工具生成字模(https://led.baud-dance.com)
 */
void OLED_PrintString(uint8_t x, uint8_t y, char *str, const Font *font, OLED_ColorMode color) {
  uint16_t i = 0;                                       // 字�?�串索引
  uint8_t oneLen = (((font->h + 7) / 8) * font->w) + 4; // 一�?字模占�?�少字节
  uint8_t found;                                        // �?否找到字�?
  uint8_t utf8Len;                                      // UTF-8编码长度
  uint8_t *head;                                        // 字模头指�?
  while (str[i]) {
    found = 0;
    utf8Len = _OLED_GetUTF8Len(str + i);
    if (utf8Len == 0) break; // 有问题的UTF-8编码

    // 寻找字�??  TODO 优化查找算法, 二分查找或者hash
    for (uint8_t j = 0; j < font->len; j++) {
      head = (uint8_t *)(font->chars) + (j * oneLen);
      if (memcmp(str + i, head, utf8Len) == 0) {
        OLED_SetBlock(x, y, head + 4, font->w, font->h, color);
        // 移动光标
        x += font->w;
        i += utf8Len;
        found = 1;
        break;
      }
    }

    // 若未找到字模,且为ASCII字�??, 则缺省显示ASCII字�??
    if (found == 0) {
      if (utf8Len == 1) {
        OLED_PrintASCIIChar(x, y, str[i], font->ascii, color);
        // 移动光标
        x += font->ascii->w;
        i += utf8Len;
      } else {
        OLED_PrintASCIIChar(x, y, ' ', font->ascii, color);
        x += font->ascii->w;
        i += utf8Len;
      }
    }
  }
}

//=======================江协字库对应函数=============================
/**
  * @brief  OLED清屏
  * @param  �?
  * @retval �?
  */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 2; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

/**
  * @brief  OLED显示一�?字�??
  * @param  Line 行位�?，范围：1~4
  * @param  Column 列位�?，范围：1~16
  * @param  Char 要显示的一�?字�?�，范围：ASCII�?见字�?
  * @retval �?
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部�?
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内�??
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部�?
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内�??
	}
}

/**
  * @brief  OLED显示一�?字�??
  * @param  Line 行位�?，范围：1~4
  * @param  Column 列位�?，范围：1~16
  * @param  Char 要显示的一�?字�?�，范围：ASCII�?见字�?
  * @retval �?
  */
void OLED_ShowALL(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部�?
	for (i = 0; i < 16; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内�??
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部�?
	for (i = 0; i < 16; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内�??
	}
}


/**
  * @brief  OLED显示字�?�串
  * @param  Line 起�?��?�位�?，范围：1~4
  * @param  Column 起�?�列位置，范围：1~16
  * @param  String 要显示的字�?�串，范围：ASCII�?见字�?
  * @retval �?
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  OLED显示数字（十进制，�?�数�?
  * @param  Line 起�?��?�位�?，范围：1~4
  * @param  Column 起�?�列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval �?
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  * @param  Line 起�?��?�位�?，范围：1~4
  * @param  Column 起�?�列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-2147483648~2147483647
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval �?
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十�?进制，�?�数�?
  * @param  Line 起�?��?�位�?，范围：1~4
  * @param  Column 起�?�列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval �?
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
  * @brief  OLED显示数字（二进制，�?�数�?
  * @param  Line 起�?��?�位�?，范围：1~4
  * @param  Column 起�?�列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval �?
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}
/**
  * @brief  OLED初�?�化
  * @param  �?
  * @retval �?
  */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//�?口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分�?�比/�?荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用�?
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始�??
	
	OLED_WriteCommand(0xA1);	//设置左右方向�?0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向�?0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚�?件配�?
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控�?
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周�?
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电�?
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开�?显示
		
	OLED_Clear();				//OLED清屏
}

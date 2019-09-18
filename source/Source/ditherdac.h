//----------------------------------------------------------------------------

//���������� ���������� ������ TDitherDac

//----------------------- ������������ �������: ------------------------------

//������������ ���������� ��� (DAC1 ��� DAC2), ����������� �����������
//�������� ���������� ����� ���������� � ���� ���������� ������������������,
//����������� Delta-Sigma ����������� ������� �������. ����������� ����� ����
//����������� �� 13 �� 24 ��� � ������� ��������� �������. ������� �������
//Delta-Sigma ���������� �������� ���������� DAC_FS. ���, ����������
//�������� � ���, ����������� �� ��� �����. ������� 12 ��� ����������� � ���,
//� ������� ���� - � Delta-Sigma ���������. ����������� ������
//DitherTable[], � ������� ���������� ����� ���� ��� � �������� �����������
//������������������ ����������. �������� �� ����� ������� � ������� DMA
//����������� � DAC � �������� ������ ����������. ������������ ������ DMA
//DMA1_Channel2 � DMA1_Channel3. ������� ������� ��������� ������ TIM3.

//----------------------------------------------------------------------------

#ifndef DITHERDAC_H
#define DITHERDAC_H

//----------------------------- ���������: -----------------------------------

#define DAC_NR           12 //Native DAC Resolution, bits
#define DAC_RES          16 //Dither DAC Resolution, bits
#define DAC_FS       200000 //Delta-Sigma Sampling Frequency, Hz
#define DAC_MAX_FINE (1 << (DAC_RES - DAC_NR)) //max fine part code
#define DAC_MAX_CODE ((1 << DAC_RES) - DAC_MAX_FINE) //max DAC code
#define DAC_BUFFER        1 //DAC buffer on/off

//----------------------------------------------------------------------------
//----------------------- ��������� ����� TDitherDac: ------------------------
//----------------------------------------------------------------------------

//DacN = 0 - DAC1, DacN = 1 - DAC2

template<uint8_t DacN>
class TDitherDac
{
private:
  TGpio<PORTA, DacN? PIN5 : PIN4> Pin_DAC; 
  uint16_t DitherTable[DAC_MAX_FINE];
public:
  TDitherDac(void) {};
  void Init(void);
  void operator = (uint16_t Value);
};

//---------------------------- �������������: --------------------------------

template<uint8_t DacN>
void TDitherDac<DacN>::Init(void)
{
  Pin_DAC.Init(IN_ANALOG);            //��������� ����� DAC 
  RCC->APB1ENR |= RCC_APB1ENR_DACEN;  //��������� ������������ DAC
  RCC->AHBENR |= RCC_AHBENR_DMA1EN;   //��������� ������������ DMA
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; //��������� ������������ TIM3
  
  TIM3->CR1 &= ~TIM_CR1_CEN;          //���������� �������
  TIM3->PSC = 0;                      //�������� ����������
  TIM3->ARR = (SYSTEM_CORE_CLOCK / DAC_FS) - 1; //������ �������
  
  if(DacN == 0) //��������� ��������, ����������� ������ ��������
  {
    DAC->CR |=
      DAC_CR_DMAEN1    * 0 | //DMA disable
      DAC_CR_MAMP1_0   * 0 | //mask/amplitude select
      DAC_CR_WAVE1_0   * 0 | //wave generation disabled
      DAC_CR_TSEL1_0   * 0 | //trigger select
      DAC_CR_TEN1      * 0 | //trigger disable
      DAC_CR_BOFF1     * DAC_BUFFER | //buffer on/off
      DAC_CR_EN1       * 1;  //DAC1 enable
    
    DMA1_Channel2->CPAR = (uint32_t)&DAC->DHR12R1; //periph. address
    DMA1_Channel2->CMAR = (uint32_t)&DitherTable;  //memory address
    DMA1_Channel2->CNDTR = DAC_MAX_FINE;           //buffer size
    
    DMA1_Channel2->CCR =
      DMA_CCR2_MEM2MEM * 0 |          //memory to memory off
      DMA_CCR2_PL_0    * 2 |          //high priority
      DMA_CCR2_MSIZE_0 * 1 |          //mem. size 16 bit
      DMA_CCR2_PSIZE_0 * 1 |          //periph. size 16 bit
      DMA_CCR2_MINC    * 1 |          //memory increment enable
      DMA_CCR2_PINC    * 0 |          //periph. increment disable
      DMA_CCR2_CIRC    * 1 |          //circular mode
      DMA_CCR2_DIR     * 1 |          //direction - from memory
      DMA_CCR2_TEIE    * 0 |          //transfer error interrupt disable
      DMA_CCR2_HTIE    * 0 |          //half transfer interrupt disable
      DMA_CCR2_TCIE    * 0 |          //transfer complete interrupt disable
      DMA_CCR2_EN      * 1;           //DMA enable
   
    TIM3->CCR3 = 0;                   //CC3 register load
    TIM3->DIER |= TIM_DIER_CC3DE;     //CC3 DMA request enable
  }
  if(DacN == 1) //��������� ��������, ����������� ������ ��������
  {
    DAC->CR |=
      DAC_CR_DMAEN2    * 0 | //DMA disable
      DAC_CR_MAMP2_0   * 0 | //mask/amplitude select
      DAC_CR_WAVE2_0   * 0 | //wave generation disabled
      DAC_CR_TSEL2_0   * 0 | //trigger select
      DAC_CR_TEN2      * 0 | //trigger disable
      DAC_CR_BOFF2     * DAC_BUFFER | //buffer on/off
      DAC_CR_EN2       * 1;  //DAC2 enable
    
    DMA1_Channel3->CPAR = (uint32_t)&DAC->DHR12R2; //periph. address
    DMA1_Channel3->CMAR = (uint32_t)&DitherTable;  //memory address
    DMA1_Channel3->CNDTR = DAC_MAX_FINE;           //buffer size
    
    DMA1_Channel3->CCR =
      DMA_CCR3_MEM2MEM * 0 |          //memory to memory off
      DMA_CCR3_PL_0    * 2 |          //high priority
      DMA_CCR3_MSIZE_0 * 1 |          //mem. size 16 bit
      DMA_CCR3_PSIZE_0 * 1 |          //periph. size 16 bit
      DMA_CCR3_MINC    * 1 |          //memory increment enable
      DMA_CCR3_PINC    * 0 |          //periph. increment disable
      DMA_CCR3_CIRC    * 1 |          //circular mode
      DMA_CCR3_DIR     * 1 |          //direction - from memory
      DMA_CCR3_TEIE    * 0 |          //transfer error interrupt disable
      DMA_CCR3_HTIE    * 0 |          //half transfer interrupt disable
      DMA_CCR3_TCIE    * 0 |          //transfer complete interrupt disable
      DMA_CCR3_EN      * 1;           //DMA enable

    TIM3->CCR4 = TIM3->ARR / 2;       //CC4 register load
    TIM3->DIER |= TIM_DIER_CC4DE;     //CC4 DMA request enable
  }
  for(uint8_t i = 0; i < DAC_MAX_FINE; i++)
    DitherTable[i] = 0;               //clear DitherTable
  TIM3->CR1 = TIM_CR1_CEN;            //timer 3 enable
}

//------------------------------ ��������: -----------------------------------

template<uint8_t DacN>
void TDitherDac<DacN>::operator = (uint16_t Value)
{
  //����������� ���������:
  if(Value > DAC_MAX_CODE) Value = DAC_MAX_CODE;
  //��������� ����� ������ � ������ �����:
  uint16_t Coarse = Value >> (DAC_RES - DAC_NR);
  uint16_t Fine = Value & (DAC_MAX_FINE - 1);
  //Delta-Sigma ���������:
  int16_t Delta, Sigma = DAC_MAX_FINE;
  uint16_t Out;
  for(uint16_t i = 0; i < DAC_MAX_FINE; i++)
  {
    //�����������:
    if(Sigma > DAC_MAX_FINE) { Delta = -DAC_MAX_FINE; Out = Coarse + 1; }
      else { Delta = 0; Out = Coarse; }
    //������������:
    Sigma = Sigma + Fine + Delta;
    //���������� �������:
    DitherTable[i] = Out;
  }
  //for(uint16_t i = 0; i < DAC_MAX_FINE; i++) //Sawtooth test
  //  DitherTable[i] = i * 16;             
  //if(DacN == 0) DAC->DHR12R1 = Value;        //Direct load test (DAC 0)
  //if(DacN == 1) DAC->DHR12R2 = Value;        //Direct load test (DAC 1) 
}

//----------------------------------------------------------------------------

#endif

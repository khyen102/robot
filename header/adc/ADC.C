void ADC_init()
{
	ADMUX = 0x00;
	ADCSRA = 0x87;
}

int get_ADC1()
{ 
	 ADMUX = 0x00; 
	 ADCSRA |= 0x40; /* 분주비 128 */ 
	 while ((ADCSRA & 0x10) == 0); 
	 return ADC; 
}
int get_ADC2()
{
	ADMUX = 0x01;
	ADCSRA |= 0x40; /* 분주비 128 */
	while ((ADCSRA & 0x10) == 0);
	return ADC;
}
int get_ADC3()
{
	ADMUX = 0x02;
	ADCSRA |= 0x40; /* 분주비 128 */
	while ((ADCSRA & 0x10) == 0);
	return ADC;
}
int get_ADC4()
{
	ADMUX = 0x03;
	ADCSRA |= 0x40; /* 분주비 128 */
	while ((ADCSRA & 0x10) == 0);
	return ADC;
}
int get_ADC5()
{
	ADMUX = 0x04;
	ADCSRA |= 0x40; /* 분주비 128 */
	while ((ADCSRA & 0x10) == 0);
	return ADC;
}
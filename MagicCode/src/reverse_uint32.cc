uint32_t Reverse(uint32_t value)
{
	value = ((value & 0xAAAAAAAA) >> 1) | ((value & 0x55555555) << 1);
	value = ((value & 0xCCCCCCCC) >> 2) | ((value & 0x33333333) << 2);
	value = ((value & 0xF0F0F0F0) >> 4) | ((value & 0x0F0F0F0F) << 4);
	value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
	value = (value >> 16) | (value << 16);

	return value;
}
void dump_struct_bytes(unsigned char* dump, int size)
{
		
	for(int i=0; i<size; i++){
		
		printf("%02x ",dump[i]);
	
		if(i%10 == 9){

			printf("\t");
			for(int j=i-9; j<i; j++){
				if(dump[j] >= 33 && dump[j] <= 127)		printf("%c ",dump[j]);
				else									printf("%c ",'.');
					
			}

			printf("\n");

		}
	}
}
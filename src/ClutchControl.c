





#include "ClutchControl.h"




// should try to avoid 64 bit where possible 

static uint32_t counterchecker = 0; 

static const uint32_t OUTPUT_OFF = (BIT30LO & BIT31LO);
static const uint32_t OUTPUT_ON_A = BIT30HI; 
static const uint32_t OUTPUT_ON_B = BIT31HI; 


static const uint32_t FRAME_OFF = (BIT28LO & BIT29LO);
static const uint32_t FRAME_ON_A = BIT28HI; 
static const uint32_t FRAME_ON_B = BIT29HI; 


// may want to make module level (static)
// uint64_t clutchStates = 0; 			// up to 64 clutches in total 

static uint32_t low_clutch_bytes = 0;
static uint32_t high_clutch_bytes = 0; 


// uint64_t prevClutchStates = 0; 
static uint32_t clutchDirections = 0;
static uint32_t springStates = 0; 


uint32_t GetSpringStates(void)
{
	return springStates; 
}



uint32_t GetClutchStatesLB(void)		
{
	return low_clutch_bytes; 
} 

uint32_t GetClutchStatesHB(void)		
{
	return high_clutch_bytes; 
} 


uint32_t GetClutchDirections(void)		
{
	return clutchDirections; 
} 

uint32_t gc(void)		
{
	return counterchecker;
} 




bool ClutchUpdate(uint32_t newSpringStates)
{
	counterchecker++;



	uint32_t prevSpringStates = springStates; // the global ome 
	springStates = newSpringStates; // update module level 

	uint32_t prevDirections = clutchDirections; // local copy 
	uint32_t newDirections = 0; 

	// even bits - frame connection
	// odd bits - output connection

	// instead of doinng all our work with 64 bit numbers, lets 
	// work on low 4 bytse and high 4 bytes 

	//uint32_t lcb_prev = low_clutch_bytes;
	//uint32_t hcb_prev = high_clutch_bytes; 

	uint32_t lcb_new = 0; 
	uint32_t hcb_new = 0; 

	// never want to write both clutches in a pair high 

	//uint32_t bitmask = BIT0HI; 

	for (uint8_t i = 0; i < 8; i++)
	{

		// deal with the low 4 bytes 

		// copy direction bits from what they where before -- change if we engage 
		newDirections |= (prevDirections << 14) & (BIT14HI | BIT15HI); 


		lcb_new &= FRAME_OFF;	// clear the bits -- may write over -- shouldnt be reqired because of bitshifts being 0 
		lcb_new &= OUTPUT_OFF;

		if (newSpringStates & BIT0HI) // frame engaged 
		{
			// frame engaged, - write bits 28 and 29  
			if (!(prevSpringStates & BIT0HI)) // if NOT previously engaged 
			{
				// toggle the direction 

				if (prevDirections & BIT0HI)
				{
					newDirections &= BIT14LO; // clear bit 
				}
				else
				{
					newDirections |= BIT14HI;	// set bit 
				}

			}

			if (newDirections & BIT14HI)
			{
				lcb_new |= FRAME_ON_A;

			}
			else
			{	
				lcb_new |= FRAME_ON_B; 
			}

		} // frame disengaged keep direction 





		if (newSpringStates & BIT1HI)
		{
			if (!(prevSpringStates & BIT1HI)) // if NOT previously engaged 
			{
				// toggle the direction 

				if (prevDirections & BIT1HI)
				{
					newDirections &= BIT15LO;
				}
				else
				{
					newDirections |= BIT15HI;
					
				}

			}


			if (newDirections & BIT15HI)
			{
				lcb_new |= OUTPUT_ON_A;

			}
			else
			{	
				lcb_new |= OUTPUT_ON_B; 
			}
		}




		// Now deal with the high bytes 
		newDirections |= (prevDirections << 14) & (BIT30HI | BIT31HI); 


		hcb_new &= FRAME_OFF;	// clear the bits -- may write over -- shouldnt be reqired because of bitshifts being 0 
		hcb_new &= OUTPUT_OFF;

		if (newSpringStates & BIT16HI) // frame engaged 
		{
			// frame engaged, - write bits 28 and 29  
			if (!(prevSpringStates & BIT16HI)) // if NOT previously engaged 
			{
				// toggle the direction 

				if (prevDirections & BIT16HI)
				{
					newDirections &= BIT30LO; // clear bit 
				}
				else
				{
					newDirections |= BIT30HI;	// set bit 
				}

			}

			if (newDirections & BIT30HI)
			{
				hcb_new |= FRAME_ON_A;

			}
			else
			{	
				hcb_new |= FRAME_ON_B; 
			}

		} // frame disengaged keep direction 





		if (newSpringStates & BIT17HI)
		{
			if (!(prevSpringStates & BIT17HI)) // if NOT previously engaged 
			{
				// toggle the direction 

				if (prevDirections & BIT17HI)
				{
					newDirections &= BIT31LO;
				}
				else
				{
					newDirections |= BIT31HI;
					
				}

			}


			if (newDirections & BIT31HI)
			{
				hcb_new |= OUTPUT_ON_A;

			}
			else
			{	
				hcb_new |= OUTPUT_ON_B; 
			}
		}

		

		if (i < 7)	// dont shift when i == 7
		{
			lcb_new >>= 4;
			hcb_new >>= 4; 
			
			//bitmask <<= 2; // left shift by 2 bits 
			newSpringStates >>= 2; 
			prevSpringStates >>= 2;

			prevDirections >>= 2; 
			newDirections >>= 2; 
		}
	}


	uint64_t clutchStates = hcb_new;
	clutchStates <<= 32;
	clutchStates |= lcb_new; 


	//sr_write64(clutchStates); 



	clutchDirections = newDirections; 
	low_clutch_bytes = lcb_new; 
	high_clutch_bytes = hcb_new; 



	return true;



}



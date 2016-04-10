//*************************************************************************************
/** @file adc.cpp
 *    This file contains a very simple A/D converter driver. This driver can read from any
 *	  A/d converter on the AtMega1281 chip.
 *
 *  Revisions:
 *    @li 01-15-2008 JRR Original (somewhat useful) file
 *    @li 10-11-2012 JRR Less original, more useful file with FreeRTOS mutex added
 *    @li 10-12-2012 JRR There was a bug in the mutex code, and it has been fixed
 *
 *  License:
 *    This file is copyright 2015 by JR Ridgely and released under the Lesser GNU 
 *    Public License, version 2. It intended for educational use only, but its use
 *    is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

#include <stdlib.h>                         // Include standard library header files
#include <avr/io.h>

#include "rs232int.h"                       // Include header for serial port class
#include "adc.h"                            // Include header for the A/D class

uint16_t sampleavg = 0;
uint16_t sample = 0;
uint16_t time = 0;

//-------------------------------------------------------------------------------------
/** \brief This constructor sets up an A/D converter. 
 *  \details The A/D is made ready so that when a  method such as @c read_once() is 
 *  called, correct A/D conversions can be performed. 
 *  @param p_serial_port A pointer to the serial port which writes debugging info. 
 */

adc::adc (emstream* p_serial_port)
{
	ptr_to_serial = p_serial_port;

	// Do A/D configuration in weird non-readable ways. The student's job is to redo 
	// this by writing good, readable, sensible code rather than obfuscated malarkey.
	//
	// Please do NOT waste time trying to reverse engineer this code; doing so is a
	// complete waste of time! It's easier and quicker to start from scratch. 
	ADMUX = 0b00000000; //clear settings
	ADMUX |= (1<<REFS0); // SETS COMPARATOR REFERENCE TO VCC
	
	ADCSRA= (0b00000000)|(1<<ADEN); // Clear settings, but enable A/D converter hardware
	ADCSRA|= (1<<ADPS0)|(1<<ADPS2); // Set A/D clock prescaler to 32
	
	
	
	// *R_D=0200;*HXD|=16<<2;
	
	// Print a handy debugging message
	DBG (ptr_to_serial, "A/D constructor OK" << endl);
}


//-------------------------------------------------------------------------------------
/** @brief   This method takes one A/D reading from the given channel and returns it. 
 *  @details This code clears the MUX Selection first, and turns off some features we don't use at first. Then it takes a single reading from the chosen channel.
 *  @param   ch The A/D channel which is being read must be from 0 to 7
 *  @return  The result of the A/D conversion
 */

uint16_t adc::read_once (uint8_t ch)
{
	// Since writing A/D code is part of an assignment, here's a well-documented piece of readable code. 
	//
	
	if(!(ch&0b00001111))DBG (ptr_to_serial, "A/D input invalid" << endl); // Check input is valid
	else ch &= 0b00001111; // Mask 4 lsb of input (just in case)
	ADMUX &= ~(1<<MUX0|1<<MUX1|1<<MUX2|1<<MUX3); //Change mux selection to zero
	ADMUX = (ADMUX|(0b00000111&ch)); //Set lower bits of ADMUX to match input
	ADMUX |= (1<<ADLAR);
	
	ADCSRA &= ~(1<<ADATE); //turn off conversion auto-triggering datasheet page 285
	ADCSRA |= (1<<ADSC); //use ADSC to start conversion, turns auto off when done.
	
	while((ADCSRA)&(1<<ADSC))
	{
	time++;
	}
	sample = (ADCH<<2);
	
	return sample;

}


//-------------------------------------------------------------------------------------
/** @brief   This takes an averaged A/D reading from a given number of samples.
 *  \details It runs a basic FOR loop in code, taking multiple readings and averaging them before returning the average input.
 *  @param   channel Which A/D channel is being read, from 0 to 7
 *  @param   samples How many samples to take from the A/D converter
 *  @return  A/D conversion result, will be between 0 and 1024.
 */

uint16_t adc::read_oversampled (uint8_t channel, uint8_t samples)
{
	DBG (ptr_to_serial, "All your readings are belong to us" << endl);
	
	sampleavg = 0;
	for(int i = 0; i < samples;i ++)
	{
		sampleavg += read_once(channel);
	}
	sampleavg /= samples;
	
	return (sampleavg);
}

//-------------------------------------------------------------------------------------
/** \brief   This overloaded operator "prints the A/D converter." 
 *  \details This overloaded operator will dump all the analog values being input to the processor.
 *			 This means it shows current readings from all the A/D channels.
 *  @param   serpt Reference to a serial port to which the printout will be printed
 *  @param   a2d   Reference to the A/D driver which is being printed
 *  @return  A reference to the same serial device on which we write information.
 *           This is used to string together things to write with @c << operators
 */

emstream& operator << (emstream& serpt, adc& a2d)
{
	// Right now this operator doesn't do anything useful. It should be made useful
	serpt << PMS ("The A/D converter registers have these inputs:") 
		  << a2d.read_oversampled (0,8) << endl
          << a2d.read_oversampled (1,8) << endl;
   

	return (serpt);
}
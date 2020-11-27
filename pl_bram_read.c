/******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include <stdlib.h>
#include "xil_io.h"
#include "xil_exception.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xscugic.h"
#include "platform.h"


/************************** Constant Definitions *****************************/
#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define INTC_DEVICE_INT_ID	XPS_FPGA0_INT_ID

/************************** Function Prototypes ******************************/
int ScuGicInterrupt_Init(u16 DeviceId);
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr);
void PmodInterruptHandler(void *CallbackRef);

/************************** Variable Definitions *****************************/

static int data_avail = 0;

struct memory_range_s {
    char *name;
    char *ip;
    unsigned long base;
    unsigned long size;
};

struct memory_range_s range = {
		"axi_bram_ctrl_0_Mem0",
		"axi_bram_ctrl_0",
		0xA0000000,
		4096,
	};

XScuGic InterruptController; 	     // Instance of the Interrupt Controller
static XScuGic_Config *GicConfig;    // Configuration parameters of the controller

int switch_addr = 0;

/******************************************************************************/
/* This is the function that is called when the interrupt occurs	*/
/******************************************************************************/
void PmodInterruptHandler(void *CallbackRef)
{
	u32 IntIDFull;
	u32 readStatus;
	int SPI_Status = 0;
	u8 *Priority;
	u8 *Trigger;
	u32 bramStartAddr = 0;

	int* read_data = (int*) malloc(100*sizeof(int));
	int* read_data_x = (int*) malloc(200*sizeof(int));
	int* read_data_y = (int*) malloc(200*sizeof(int));
	int* read_data_z = (int*) malloc(200*sizeof(int));


	if (switch_addr == 1)
		bramStartAddr = 96;
	else
		bramStartAddr = 0;

	// Read data from AXI BRAM controller
	for (int i = 0; i < 96; i++)
    {
		*(read_data + i) = Xil_In32 ( (INTPTR)(range.base + (bramStartAddr + i)*sizeof(u32)) );
	}

	// Split the bytes read into X, Y, Z 16-bit unsigned integer values
	for (int i = 0; i < 32; i++)
    {
		*(read_data_x + i) = *(read_data + i);	// Read from PL BRAM
		print("X = "); putnum((u32) *(read_data_x + i)); print("\n\r");	// UART Write
		*(read_data_y + i) = *(read_data + i + 32);	// Read from PL BRAM
		print("Y = "); putnum((u32) *(read_data_y + i)); print("\n\r");	// UART Write
		*(read_data_z + i) = *(read_data + i + 64);	// Read from PL BRAM
		print("Z = "); putnum((u32) *(read_data_z + i)); print("\n\r");	// UART Write
    }

	data_avail = 1;
	if (switch_addr == 0)
		switch_addr = 1;
	else
		if (switch_addr == 1)
			switch_addr = 0;

}

/******************************************************************************/
/* Connect interrupt controller to the interrupt-handling hardware	*/
/******************************************************************************/
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr)
{

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler,	XScuGicInstancePtr);

	return XST_SUCCESS;
}

/******************************************************************************/
/* Initialize the GIC */
/******************************************************************************/
int ScuGicInterrupt_Init(u16 DeviceId)
{
	int Status;

	//Initialize the interrupt controller driver
	GicConfig = XScuGic_LookupConfig(DeviceId);
	if (NULL == GicConfig)
	{
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig, GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(&InterruptController, INTC_DEVICE_INT_ID, 0xA0, 0x3);

	// Setup the Interrupt System
	Status = SetUpInterruptSystem(&InterruptController);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	/* Connect a device driver handler that will be called when an
	* interrupt for the device occurs, the device driver handler performs
	* the specific interrupt processing for the device
	*/
	Status = XScuGic_Connect(&InterruptController, INTC_DEVICE_INT_ID,
						(Xil_ExceptionHandler)PmodInterruptHandler, (void *)&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Enable the interrupt
	XScuGic_Enable(&InterruptController, INTC_DEVICE_INT_ID);

	// Enable exception handling
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}


/******************************************************************************/
/* Main function	*/
/******************************************************************************/
int main()
{
    init_platform();

	int xStatus;

	//SCUGIC interrupt controller Initialization
	xStatus = ScuGicInterrupt_Init(INTC_DEVICE_ID);
	if(XST_SUCCESS != xStatus)
		print(" :( SCUGIC INIT FAILED \n\r");

	//Wait For interrupt
	while(1)
	{
		if (data_avail == 1)
		{
			data_avail = 0;
		}
	}

    cleanup_platform();
    return 0;
}

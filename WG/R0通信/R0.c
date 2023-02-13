#include <ntifs.h>

#include <ntddk.h>



#define DEVICE_NAME L"\\device\\PAPAYADEVICE"
#define SYMBOL_NAME_LINK L"\\??\\papayadevice"

#define CODE_READ CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CODE_WRITE CTL_CODE(FILE_DEVICE_UNKNOWN,0x900,METHOD_BUFFERED,FILE_ANY_ACCESS)

PEPROCESS Global_Peprocess = NULL;

NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);
NTKERNELAPI CHAR* PsGetProcessImageFileName(PEPROCESS Process);
NTSTATUS NTAPI MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);

NTSTATUS KeReadProcessMemory(PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	__try
	{
		PEPROCESS TargetProcess = PsGetCurrentProcess();
		SIZE_T Result;
		if (NT_SUCCESS(MmCopyVirtualMemory(Global_Peprocess, SourceAddress, TargetProcess, TargetAddress, Size, KernelMode, &Result)))
			return STATUS_SUCCESS;
		else
			return STATUS_ACCESS_DENIED;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return STATUS_ACCESS_DENIED;
	}
	return STATUS_ACCESS_DENIED;
}

VOID DriverUnload(PDRIVER_OBJECT pDriver) {
	UNICODE_STRING symbolNameLink;
	RtlInitUnicodeString(&symbolNameLink, SYMBOL_NAME_LINK);
	IoDeleteSymbolicLink(&symbolNameLink);
	IoDeleteDevice(pDriver->DeviceObject);

	//DbgPrint("driver unload\n");
	KdPrintEx((77, 0, "driver unload\n"));
}
NTSTATUS DeviceCreate(_In_ struct _DEVICE_OBJECT* DeviceObject, _Inout_ struct _IRP* Irp) {
	//DbgPrint("device link\n");
	KdPrintEx((77, 0, "devide link\n"));
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = STATUS_SUCCESS;
	IofCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
NTSTATUS DeviceClose(_In_ struct _DEVICE_OBJECT* DeviceObject, _Inout_ struct _IRP* Irp) {
	//DbgPrint("devide close\n");
	KdPrintEx((77, 0, "devide close\n"));
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = STATUS_SUCCESS;
	IofCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

ULONG pid = 0;
ULONG readAddress = 0;
ULONG readLen = 0;

NTSTATUS DeviceDispatch(_In_ struct _DEVICE_OBJECT* DeviceObject, _Inout_ struct _IRP* Irp) {
	PIO_STACK_LOCATION psl = IoGetCurrentIrpStackLocation(Irp);
	ULONG code = psl->Parameters.DeviceIoControl.IoControlCode;
	PVOID systemBuf = Irp->AssociatedIrp.SystemBuffer;
	ULONG inLen = psl->Parameters.DeviceIoControl.InputBufferLength;
	ULONG outLen = psl->Parameters.DeviceIoControl.OutputBufferLength;


	char buf[0x1000];
	NTSTATUS status;

	switch (code) {
	case CODE_READ:
		//DbgPrint("CODE_READ %X\n", CODE_READ);
		KdPrintEx((77, 0, "CODE_READ %X\n", CODE_READ));
		if (pid == 0 || readAddress == 0 || readLen == 0) {
			//DbgPrint("CODE_WRITE run unsuccessed \n", CODE_WRITE);
			KdPrintEx((77, 0, "CODE_WRITE run unsuccessed \n", CODE_WRITE));
			Irp->IoStatus.Information = 0;
			Irp->IoStatus.Status = STATUS_SUCCESS;
			IofCompleteRequest(Irp, IO_NO_INCREMENT);
			return STATUS_SUCCESS;
		}

		status = PsLookupProcessByProcessId((HANDLE)pid, &Global_Peprocess);
		if (!NT_SUCCESS(status)) {
			//DbgPrint("PsLookupProcessByProcessId error\n");
			KdPrintEx((77, 0, "PsLookupProcessByProcessId error\n"));
			return status;
		}
		KeReadProcessMemory((PVOID)readAddress, (PVOID)&buf, readLen);

		memcpy(systemBuf, buf, readLen);
		Irp->IoStatus.Information = readLen;
		break;
	case CODE_WRITE:
		//DbgPrint("CODE_WRITE %X\n", CODE_WRITE);
		KdPrintEx((77, 0, "CODE_WRITE %X\n", CODE_WRITE));
		int* int_ptr = (int*)systemBuf;
		pid = (ULONG)(*(int_ptr + 0));
		readAddress = (ULONG)(*(int_ptr + 1));
		readLen = (ULONG)(*(int_ptr + 2));
		//DbgPrint("pid %d   readAddress %p  readLen %d \n", readAddress,readLen);
		KdPrintEx((77, 0, "pid %d   readAddress %p  readLen %d \n",pid, readAddress, readLen));
		Irp->IoStatus.Information = 0;
		break;
	}
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IofCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	UNICODE_STRING deviceName;
	UNICODE_STRING symbolNameLink;
	PDRIVER_OBJECT pdriverobj;
	NTSTATUS status = STATUS_SUCCESS;
	pDriver->DriverUnload = DriverUnload;
	//DbgPrint("driver load\n");
	KdPrintEx((77, 0, "driver load\r\n"));
	RtlInitUnicodeString(&deviceName, DEVICE_NAME);
	RtlInitUnicodeString(&symbolNameLink, SYMBOL_NAME_LINK);
	status = IoCreateDevice(pDriver, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, TRUE, &pdriverobj);
	if (!NT_SUCCESS(status)) {
		//DbgPrint("create device error\n");
		KdPrintEx((77, 0, "create device error\n"));
		return status;
	}

	status = IoCreateSymbolicLink(&symbolNameLink, &deviceName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(pdriverobj);
		//DbgPrint("create device link error\n");
		KdPrintEx((77, 0, "create device link error\n"));
		return status;
	}

	pDriver->Flags |= DO_BUFFERED_IO;
	pDriver->MajorFunction[IRP_MJ_CREATE] = DeviceCreate;
	pDriver->MajorFunction[IRP_MJ_CLOSE] = DeviceClose;
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceDispatch;
	

	return STATUS_SUCCESS;
}
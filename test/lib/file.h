FileOpen:		; (filename, mode)
				; Opens a file (mode: 0 = append, 1 = read, 2 = truncate)
				; Returns handle or -1 on error
#if SCI_VERSION < SCI_VERSION_01
			push0
			&rest 1
			callk k_FOpen 0
#else
			push1
			pushi f_FileIO_Open
			&rest 1
			callk k_FileIO 2
#endif
			ret

FileGets:		; (buf, size, handle)
				; Reads line from input file
				; Returns number of characters read
#if  SCI_VERSION < SCI_VERSION_01
			push0
			&rest 1
			callk k_FGets 0
#else
			pushi 1
			pushi f_FileIO_ReadString
			&rest 1
			callk k_FileIO 2
#endif
			ret

FilePuts:		; (handle, buf)
				; Writes line to output file
				; SCI1: Returns 0 on success, error code otherwise
#if SCI_VERSION < SCI_VERSION_01
			push0
			&rest 1
			callk k_FPuts 0
#else
			push1
			pushi f_FileIO_WriteString
			&rest 1
			callk k_FileIO 2
#endif
			ret

FileClose:		; (handle)
				; Closes a file
#if SCI_VERSION < SCI_VERSION_01
			push0
			&rest 1
			callk k_FClose 0
#else
			push1
			pushi f_FileIO_Close
			&rest 1
			callk k_FileIO 2
#endif
			ret

//182(0xb6)
TSS_EXPORT TSS_Error tss_broadcastSynchronizationPulse(TSS_Device_Id device, unsigned int * timestamp){
    return writeRead(device, &simple_commands[TSS_BROADCAST_SYNCHRONIZATION_PULSE],NULL,NULL,timestamp);
}



TSS_Error writeRead(TSS_Device_Id device, const TSS_Command * cmd_info,   //TODO add typecheck here
              const char * input_data, char * output_data,
              unsigned int *timestamp){
//    printf("serial device: %X\n",device);
    unsigned int tss_idx;
    unsigned int tss_d_idx;
    unsigned int i;
    int result = TSS_INVALID_ID;
    if(!(device & cmd_info->compatibility_mask)){
        printf("Invalid command for that sensor type!!!\n");
        return TSS_INVALID_COMMAND;
    }
    #ifdef DEBUG_PRINT
    printf("device & TSS_ALL_SENSORS_ID = %x\n", (device & TSS_ALL_SENSORS_ID));
    printf("device  = %x\n", (device ));
    #endif
    switch(device & TSS_ALL_SENSORS_ID){
    case(TSS_DONGLE_ID):
        #ifdef DEBUG_PRINT
        printf("dong write.....\n");
        #endif
        tss_idx = (device-TSS_DONGLE_ID);
        if(tss_idx < dongle_list_len && dongle_list[tss_idx]){
            if( dongle_list[tss_idx]->fw_compatibility < cmd_info->fw_compatibility){
                return TSS_ERROR_FIRMWARE_INCOMPATIBLE;
            }
            #ifdef DEBUG_PRINT
            printf("Dongle description_str: %s\n",cmd_info->description_str);
            #endif
            return f9WriteReadDongle(dongle_list[tss_idx],
                                cmd_info, input_data, output_data, timestamp); //*****this one*************************************timestamp
        }
        break;
    case(TSS_WIRELESS_W_ID):
        tss_idx = (device-TSS_WIRELESS_W_ID);
        if(tss_idx < sensor_list_len && sensor_list[tss_idx]){
            tss_d_idx = (sensor_list[tss_idx]->dongle-TSS_DONGLE_ID);
            if(tss_d_idx < dongle_list_len && dongle_list[tss_d_idx]){
                if( dongle_list[tss_d_idx]->fw_compatibility < cmd_info->fw_compatibility){
                    return TSS_ERROR_FIRMWARE_INCOMPATIBLE;
                }
                if( sensor_list[tss_idx]->fw_compatibility < cmd_info->fw_compatibility){
                    return TSS_ERROR_FIRMWARE_INCOMPATIBLE;
                }
                #ifdef DEBUG_PRINT
                printf("Wireless description_str: %s\n",cmd_info->description_str);
                #endif
                for(i=0 ; i < wireless_retries ; i++){
                    result= faWriteReadDongle( dongle_list[tss_d_idx],
                                            sensor_list[tss_idx]->logical_id,
                                            cmd_info, input_data, output_data,
                                            timestamp); //******************************************timestamp
                    if(result == TSS_NO_ERROR){
                        break;
                    }
                }
                return result;
            }
        }
        break;
    case(TSS_USB_ID):
    case(TSS_EMBEDDED_ID):
    case(TSS_WIRELESS_ID):
    case(TSS_DATALOGGER_ID):
    case(TSS_BLUETOOTH_ID):
        tss_idx = ~(~device|TSS_NO_DONGLE_ID);
    //    printf("serial idx: %X\n",tss_idx);
        if(tss_idx < sensor_list_len && sensor_list[tss_idx]){
            if( sensor_list[tss_idx]->fw_compatibility < cmd_info->fw_compatibility){
                return TSS_ERROR_FIRMWARE_INCOMPATIBLE;
            }
            #ifdef DEBUG_PRINT
            printf("description_str: %s\n",cmd_info->description_str);
            #endif
            return f9WriteRead(sensor_list[tss_idx],
                               cmd_info, input_data, output_data, timestamp); //********f9readwrite**********************************timestamp
        }
    }
    return TSS_INVALID_ID;
}




//this one
TSS_Error f9WriteReadDongle(TSS_Dongle * dongle, const TSS_Command * cmd_info,
                const char * input_data, char * output_data,
                unsigned int * timestamp){
    unsigned char start_byte = 0xf9;
    unsigned int write_size = cmd_info->in_data_len+3; //3 ={Start byte, Command byte, Checksum byte}
//    unsigned int read_size =  cmd_info->rtn_data_len;
    char write_array[256];
    DWORD num_bytes_written;
//    DWORD num_bytes_read;
    //TSS_Header_85 header_packet;
    OVERLAPPED osWrite= {0};
    //write_array = malloc(write_size);
    //if( write_array == 0){printf("ERROR: Out of memory\n"); return TSS_ERROR_MEMORY;}
    DWORD dwWaitResult;
    write_array[0]=start_byte;
    write_array[1]=cmd_info->command;
    if(cmd_info->in_data_len){
        memcpy(write_array+2,input_data, cmd_info->in_data_len);
        parseData(write_array+2,cmd_info->in_data_len, cmd_info->in_data_detail);
    }
    write_array[write_size-1] = createChecksum(write_array+1, write_size-2);
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    EnterCriticalSection(&dongle->reader_lock);
    #ifdef _HEXDUMP
    _hexDump(write_array,write_size, "<<");
    #endif
    if(!WriteFile(  dongle->serial_port,write_array,write_size,
                    &num_bytes_written,
                    &osWrite)){
        if (GetLastError() != ERROR_IO_PENDING){
            #ifdef DEBUG_PRINT
            printf ("Error writing to port\n");
            #endif
            //free(write_array);
            CloseHandle(osWrite.hEvent);
            LeaveCriticalSection(&dongle->reader_lock);
            return TSS_ERROR_WRITE;
        }
    }
    //free(write_array);
    if (!GetOverlappedResult(dongle->serial_port, &osWrite, &num_bytes_written, TRUE)){
        #ifdef DEBUG_PRINT
        printf ("Error writing to port(%d)\n", (unsigned int)GetLastError());;
        #endif
        CloseHandle(osWrite.hEvent);
        LeaveCriticalSection(&dongle->reader_lock);
        return TSS_ERROR_WRITE;
    }
    CloseHandle(osWrite.hEvent);

    dwWaitResult=WaitForSingleObject(dongle->writer_event,1000);
    if(dwWaitResult ==WAIT_OBJECT_0){
        TSS_Header_87 * header_packet = (TSS_Header_87 *)dongle->last_header_data;
        if(timestamp){
            *timestamp = header_packet->timestamp;
        }
//        printf("header_packet %u, %u, %X, %u\n",header_packet->success_failure,
//                            header_packet->logical_id,
//                              header_packet->command_echo,
//                              header_packet->data_length);
       if(header_packet->success_failure){
            SetEvent(dongle->reader_event);
            LeaveCriticalSection(&dongle->reader_lock);
            return TSS_ERROR_COMMAND_FAIL;
        }
        if(cmd_info->rtn_data_len){
            memcpy(output_data,dongle->last_out_data,cmd_info->rtn_data_len);
        }
        SetEvent(dongle->reader_event);
        LeaveCriticalSection(&dongle->reader_lock);
        parseData(output_data,cmd_info->rtn_data_len, cmd_info->rtn_data_detail);
        return TSS_NO_ERROR;
    }
    printf("Wait error f9WriteRead(%d)\n", (unsigned int)GetLastError());
    LeaveCriticalSection(&dongle->reader_lock);
    return TSS_ERROR_TIMEOUT;
}












TSS_Error faWriteReadDongle(TSS_Dongle * dongle, char logical_id, const TSS_Command * cmd_info,
                const char * input_data, char * output_data,
                unsigned int * timestamp){
    unsigned char start_byte = 0xfa;
    unsigned int write_size = cmd_info->in_data_len+4; //3 ={Start byte, logical_id, Command byte, Checksum byte}
//    unsigned int read_size =  cmd_info->rtn_data_len;
    char write_array[256];
    DWORD num_bytes_written;
//    DWORD num_bytes_read;
    //TSS_Header_85 header_packet;
    OVERLAPPED osWrite= {0};
    DWORD dwWaitResult;
    //write_array = malloc(write_size);
    //if( write_array == 0){printf("ERROR: Out of memory\n"); return TSS_ERROR_MEMORY;}
    write_array[0]=start_byte;
    write_array[1]=logical_id;
    write_array[2]=cmd_info->command;
    if(cmd_info->in_data_len){
        memcpy(write_array+3,input_data, cmd_info->in_data_len);
        parseData(write_array+3,cmd_info->in_data_len, cmd_info->in_data_detail);
    }
    write_array[write_size-1] = createChecksum(write_array+1, write_size-2);
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    EnterCriticalSection(&dongle->reader_lock);
    #ifdef _HEXDUMP
    _hexDump(write_array,write_size, "<<");
    #endif
    if(!WriteFile(  dongle->serial_port,write_array,write_size,
                    &num_bytes_written,
                    &osWrite)){
        if (GetLastError() != ERROR_IO_PENDING){
            #ifdef DEBUG_PRINT
            printf ("Error writing to port\n");
            #endif
            //free(write_array);
            CloseHandle(osWrite.hEvent);
            LeaveCriticalSection(&dongle->reader_lock);
            return TSS_ERROR_WRITE;
        }
    }
    //free(write_array);
    if (!GetOverlappedResult(dongle->serial_port, &osWrite, &num_bytes_written, TRUE)){
        #ifdef DEBUG_PRINT
        printf ("Error writing to port(%d)\n", (unsigned int)GetLastError());;
        #endif
        CloseHandle(osWrite.hEvent);
        LeaveCriticalSection(&dongle->reader_lock);
        return TSS_ERROR_WRITE;
    }
    CloseHandle(osWrite.hEvent);

    dwWaitResult=WaitForSingleObject(dongle->writer_event,1000);
    if(dwWaitResult ==WAIT_OBJECT_0){
        TSS_Header_87 * header_packet = (TSS_Header_87 *)dongle->last_header_data;
        if(timestamp){
            *timestamp = header_packet->timestamp;
        }
//        printf("header_packet %u, %u, %X, %u\n",header_packet->success_failure,
//                            header_packet->logical_id,
//                              header_packet->command_echo,
//                              header_packet->data_length);
       if(header_packet->success_failure){
            SetEvent(dongle->reader_event);
            LeaveCriticalSection(&dongle->reader_lock);
            return TSS_ERROR_COMMAND_FAIL;
        }
        if(cmd_info->rtn_data_len){
            memcpy(output_data,dongle->last_out_data,cmd_info->rtn_data_len);
        }
        SetEvent(dongle->reader_event);
        LeaveCriticalSection(&dongle->reader_lock);
        parseData(output_data,cmd_info->rtn_data_len, cmd_info->rtn_data_detail);
        return TSS_NO_ERROR;
    }
    printf("Wait error faWriteRead(%d)\n", (unsigned int)GetLastError());
    LeaveCriticalSection(&dongle->reader_lock);
    return TSS_ERROR_TIMEOUT;
}
















TSS_Error f9WriteRead(TSS_Sensor * sensor, const TSS_Command * cmd_info,
                const char * input_data, char * output_data,
                unsigned int * timestamp){
    unsigned char start_byte = 0xf9;
    unsigned int write_size = cmd_info->in_data_len+3; //3 ={Start byte, Command byte, Checksum byte}
//    unsigned int read_size =  cmd_info->rtn_data_len;
    char * write_array;
    DWORD num_bytes_written;
//    DWORD num_bytes_read;
    //TSS_Header_69 header_packet;
    OVERLAPPED osWrite= {0};
    DWORD dwWaitResult;
    write_array = malloc(write_size);
    if( write_array == 0){printf("ERROR: Out of memory\n"); return TSS_ERROR_MEMORY;}
    write_array[0]=start_byte;
    write_array[1]=cmd_info->command;
    if(cmd_info->in_data_len){
        memcpy(write_array+2,input_data, cmd_info->in_data_len);
        parseData(write_array+2,cmd_info->in_data_len, cmd_info->in_data_detail);
    }
    write_array[write_size-1] = createChecksum(write_array+1, write_size-2);
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    EnterCriticalSection(&sensor->reader_lock);
    #ifdef _HEXDUMP
    _hexDump(write_array,write_size, "<<");
    #endif
    if(!WriteFile(  sensor->serial_port,write_array,write_size,
                    &num_bytes_written,
                    &osWrite)){
        if (GetLastError() != ERROR_IO_PENDING){
            #ifdef DEBUG_PRINT
            printf ("Error writing to port\n");
            #endif
            free(write_array);
            CloseHandle(osWrite.hEvent);
            LeaveCriticalSection(&sensor->reader_lock);
            return TSS_ERROR_WRITE;
        }
    }
    #ifdef DEBUG_PRINT
    printf("wrote data\n");
    #endif
    free(write_array);
    if (!GetOverlappedResult(sensor->serial_port, &osWrite, &num_bytes_written, TRUE)){
        #ifdef DEBUG_PRINT
        printf ("Error writing to port(%d)\n", (unsigned int)GetLastError());
        #endif
        CloseHandle(osWrite.hEvent);
        LeaveCriticalSection(&sensor->reader_lock);
        return TSS_ERROR_WRITE;
    }
    CloseHandle(osWrite.hEvent);
    dwWaitResult=WaitForSingleObject(sensor->writer_event,1000);
    if(dwWaitResult ==WAIT_OBJECT_0){
        TSS_Header_71 * header_packet = (TSS_Header_71 *)sensor->last_header_data;
//        printf("header_packet %u, %X, %u\n",header_packet->success_failure,
//                                            header_packet->command_echo,
//                                            header_packet->data_length);
        if(timestamp){
            *timestamp = header_packet->timestamp;
        }
        if(header_packet->success_failure){
            SetEvent(sensor->reader_event);
            LeaveCriticalSection(&sensor->reader_lock);
            return TSS_ERROR_COMMAND_FAIL;
        }
        if(cmd_info->rtn_data_len){
            memcpy(output_data,sensor->last_out_data, cmd_info->rtn_data_len);
        }
        SetEvent(sensor->reader_event);
        LeaveCriticalSection(&sensor->reader_lock);
        parseData(output_data,cmd_info->rtn_data_len, cmd_info->rtn_data_detail);
        return TSS_NO_ERROR;
    }
    // An error occurred
    printf("Wait error f9WriteRead(%d)\n", (unsigned int)GetLastError());
    LeaveCriticalSection(&sensor->reader_lock);
    return TSS_ERROR_TIMEOUT;
}











#include "transfer.h"

int frame_header_init(unsigned char type, unsigned char sub_type,\
                        frame_header *header)
{

    switch(type)
    {
    case FRAME_TYPE_HEARTBEAT:
        break;
    case FRAME_TYPE_CONTROL:
        switch (sub_type)
        {
        case FRAME_CONTROL_LOGIN:
            
            header->type = FRAME_TYPE_CONTROL;
            header->sub_type = FRAME_CONTROL_LOGIN;
            header->crc = 0;
            header->length = 0;

            return 0;
        case FRAME_CONTROL_FILE_INFO:
            break;
        default:
            t_log("frame sub type unknown");
            return -1;
        }
        break;
    case FRAME_TYPE_DATA:
        break;
    default:
        t_log("frame type unknown");
        return -1;
    }


}

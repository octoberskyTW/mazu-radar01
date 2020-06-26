#!/usr/bin/python3 
import sys
import os
import argparse
import binascii
import struct

MMWDEMO_OUTPUT_MSG_DETECTED_POINTS=1
MMWDEMO_OUTPUT_MSG_DETECTED_POINTS_SIDE_INFO=7

def radar01_Cartesian_info_dump(frameNum, points_dict, points_side_dict):
    info = ""
    for p in points_dict:
        info = "{frame},{idx},{x:6.5f},{y:6.5f},{z:6.5f},{vel:6.5f},".format(frame=frameNum, idx=p,
               x=points_dict[p][0], y=points_dict[p][1], z=points_dict[p][2], vel=points_dict[p][3])
        info += "{snr},{noise}".format(snr=points_side_dict[p][0], noise=points_side_dict[p][1])
        print(info)
    
# struct format: https://docs.python.org/3/library/struct.html
def decode_tlv_type(rx_buff, frameNum, numDetectedObj, numTLVs):
    size = len(rx_buff)
    points_dict = {}
    points_side_dict = {}
    while numTLVs != 0 and size > 0:
        # Get TLV info
        fmt = "<II"
        tlv_type, tlv_len = struct.unpack_from(fmt, rx_buff[0:8])
        #print("TLV: type {}, len {}".format(tlv_type, tlv_len))
        rx_buff = rx_buff[8:]
        size = size - 8
        # Handle TLV content
        if tlv_type == MMWDEMO_OUTPUT_MSG_DETECTED_POINTS:
            for x in range(0,numDetectedObj):
                fmt='<ffff'
                l = struct.calcsize(fmt)
                obj_tuple = struct.unpack_from(fmt, rx_buff[0:l])
                key="obj_"+str(x)
                points_dict[key] = obj_tuple
                rx_buff = rx_buff[l:]
        elif tlv_type == MMWDEMO_OUTPUT_MSG_DETECTED_POINTS_SIDE_INFO:
            for x in range(0,numDetectedObj):
                fmt='<hh'
                l = struct.calcsize(fmt)
                obj_tuple = struct.unpack_from(fmt, rx_buff[0:l])
                key="obj_"+str(x)
                points_side_dict[key] = obj_tuple
                rx_buff = rx_buff[l:]
        else:
            rx_buff = rx_buff[tlv_len:]
        numTLVs = numTLVs - 1
        size = size - tlv_len
    radar01_Cartesian_info_dump(frameNum, points_dict, points_side_dict)


def process_message_from_file(input_file, fsize):
    '''
    Parse the Binary to ASCII data
    '''
    offset=0
    magic_bytes=bytes(b'\x02\x01\x04\x03\x06\x05\x08\x07')
    input_file.seek(0)
    rx_buff = input_file.read(40)
    print("Frame Seq, Obj_index, x, y, z, velocity, snr, noise");
    while offset < fsize:
        input_file.seek(offset)
        rx_buff = input_file.read(40)
        if(magic_bytes == rx_buff[0:8]):
            # magicWord, 0123
            # version, 4
            # totalPacketLen, 5
            # platform, 6
            # frameNumber, 7
            # timeCpuCycles, 8
            # numDetectedObj, 9
            # numTLVs, 10
            # subFrameNumber, 11
            fmt = "<4HIIIIIIII"
            header_tuple = struct.unpack_from(fmt, rx_buff[0:40])
            totalPacketLen=header_tuple[5]
            frameNumber=header_tuple[7]
            numDetectedObj=header_tuple[9]
            numTLVs=header_tuple[10]
            print(f'# Frame {frameNumber}: DetectedObjs={numDetectedObj} : numTLVs={numTLVs}')
            offset = offset + 40
            input_file.seek(offset)
            # All data put into the rx_buff
            rx_buff = input_file.read(totalPacketLen - 40)
            decode_tlv_type(rx_buff, frameNumber, numDetectedObj, numTLVs)
            offset = offset + totalPacketLen - 40
        else:
            offset = offset + 1
    return



def main():
    welcome = "Mazu-Radar01 Toolbox"
    parser = argparse.ArgumentParser(description=welcome)
    parser.add_argument('--infile', '-f',
                        help='Input file name or path')
    parser.parse_args()
    args = parser.parse_args()
    infile = args.infile
    if(infile):
        with open(infile, "rb") as input_binary:
            fsize = os.path.getsize(infile)
            process_message_from_file(input_binary, fsize)

if __name__ == "__main__":
    main()

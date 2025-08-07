package main

import (
    "bytes"
    "encoding/binary"
    "fmt"
    "log"
    "net"
    "strconv"
    "unsafe"
)

const (
    MAGIC    = "AB" // 2字节字符数组
    VERSION  = 0x0101
    CMD_TEST = 0x0001
)

type Header struct {
    Magic    [2]byte
    Version  uint16
    Command  uint16
    BodyLen  uint32
    Checksum uint32
}

type Body struct {
    SeqNum    uint32
    Timestamp uint64
    Data      [4]byte
}

func main() {
    conn, err := net.Dial("tcp", "localhost:8080")
    if err != nil {
        log.Fatal("连接失败:", err)
    }
    defer conn.Close()

    // 准备协议体
    body := Body{
        SeqNum:    0x11223344,
        Timestamp: 0x5566778899AABBCC,
        Data:      [4]byte{'T', 'E', 'S', 'T'},
    }

    // 序列化协议体
    bodyBuf := new(bytes.Buffer)
    binary.Write(bodyBuf, binary.BigEndian, body.SeqNum)
    binary.Write(bodyBuf, binary.BigEndian, body.Timestamp)
    bodyBuf.Write(body.Data[:])

    // 准备协议头
    header := Header{
        Magic:    [2]byte{MAGIC[0], MAGIC[1]},
        Version:  VERSION,
        Command:  CMD_TEST,
        BodyLen:  uint32(bodyBuf.Len()),
    }
    // 虽然此处header打印16字节（也有对齐补齐），但下面传输的二进制是手动指定了二进制流
    log.Printf("header len:%d, bodyBuf len:%d", unsafe.Sizeof(Header{}), bodyBuf.Len())

    // 计算校验和（基于大端序数据计算）
    checksum := calculateChecksum(bodyBuf.Bytes())
    header.Checksum = checksum

    // 序列化协议头
    headerBuf := new(bytes.Buffer)
    headerBuf.Write(header.Magic[:]) // 直接写入字节数组
    binary.Write(headerBuf, binary.BigEndian, header.Version)
    binary.Write(headerBuf, binary.BigEndian, header.Command)
    binary.Write(headerBuf, binary.BigEndian, header.BodyLen)
    binary.Write(headerBuf, binary.BigEndian, header.Checksum)
    log.Printf("req headerBuf len:%d, bodyBuf len:%d", headerBuf.Len(), bodyBuf.Len())

    // 组合完整报文
    fullPacket := append(headerBuf.Bytes(), bodyBuf.Bytes()...)

    // 发送数据
    if _, err := conn.Write(fullPacket); err != nil {
        log.Fatal("发送失败:", err)
    }

    fmt.Println("发送成功:")
    printPacket(header, body, fullPacket)
}

func calculateChecksum(data []byte) uint32 {
    var sum uint32
    for _, b := range data {
        sum += uint32(b)
    }
    return sum
}

func printPacket(header Header, body Body, raw []byte) {
    fmt.Printf("Header:\n")
    fmt.Printf("  Magic:    %s (0x%X)\n", string(header.Magic[:]), header.Magic)
    fmt.Printf("  Version:  0x%04X\n", header.Version)
    fmt.Printf("  Command:  0x%04X\n", header.Command)
    fmt.Printf("  BodyLen:  %d\n", header.BodyLen)
    fmt.Printf("  Checksum: 0x%08X\n", header.Checksum)

    fmt.Printf("\nBody:\n")
    fmt.Printf("  SeqNum:   0x%08X\n", body.SeqNum)
    fmt.Printf("  Timestamp:0x%016X\n", body.Timestamp)
    fmt.Printf("  Data:     %s\n", strconv.Quote(string(body.Data[:])))

    fmt.Printf("\nRaw Bytes (%d bytes):\n", len(raw))
    for i := 0; i < len(raw); i += 8 {
        end := i + 8
        if end > len(raw) {
            end = len(raw)
        }
        fmt.Printf("  [%04X] % X\n", i, raw[i:end])
    }
}

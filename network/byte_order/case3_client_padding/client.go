package main

import (
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
    Magic    [2]byte  // 0-1
    Version  uint16   // 2-3 (紧接Magic，不需要填充)
    Command  uint16   // 4-5
    _        [2]byte  // 6-7: 2字节填充，和C一致
    BodyLen  uint32   // 8-11
    Checksum uint32   // 12-15
} // 总大小16字节

// 而不是：
// type Header struct {
//     Magic    [2]byte  // 2字节
//     _        [2]byte  // 填充2字节：使Version从4字节开始（假设4字节对齐）
//     Version  uint16   // 2字节
//     Command  uint16   // 2字节
//     BodyLen  uint32   // 4字节
//     Checksum uint32   // 4字节
// }

type Body struct {
    SeqNum    uint32   // 0-3
    _         [4]byte  // 4-7: 填充（使Timestamp对齐到8字节）
    Timestamp uint64   // 8-15
    Data      [4]byte  // 16-19
    _         [4]byte  // 20-23: 填充（使总大小为24字节），和C++补齐对应
}

// 序列化时手动处理填充
func serializeHeader(h Header) []byte {
    buf := make([]byte, 16)
    // 按实际偏移量写入
    copy(buf[0:2], h.Magic[:])          // 0-1
    binary.BigEndian.PutUint16(buf[2:4], h.Version)  // 2-3
    binary.BigEndian.PutUint16(buf[4:6], h.Command)  // 4-5
    // 6-7 为填充区（保留为0）
    binary.BigEndian.PutUint32(buf[8:12], h.BodyLen) // 8-11
    binary.BigEndian.PutUint32(buf[12:16], h.Checksum) // 12-15
    return buf
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
    // bodyBuf := new(bytes.Buffer)
    // binary.Write(bodyBuf, binary.BigEndian, body.SeqNum)
    // binary.Write(bodyBuf, binary.BigEndian, body.Timestamp)
    // bodyBuf.Write(body.Data[:])
    // 也按对齐补齐填充后的偏移来设置数据，和服务端保持一致
    bodyBuf := make([]byte, 24)
    binary.BigEndian.PutUint32(bodyBuf[0:4], body.SeqNum)
    binary.BigEndian.PutUint64(bodyBuf[8:16], body.Timestamp)
    copy(bodyBuf[16:20], body.Data[:])

    // 准备协议头
    header := Header{
        Magic:    [2]byte{MAGIC[0], MAGIC[1]},
        Version:  VERSION,
        Command:  CMD_TEST,
		BodyLen:  (uint32)(len(bodyBuf)),
    }
    // 虽然此处header打印16字节（也有对齐补齐），但下面传输的二进制是手动指定了二进制流
    log.Printf("Header len:%d, Body len:%d", unsafe.Sizeof(Header{}), unsafe.Sizeof(Body{}))

    // 计算校验和（基于大端序数据计算）
    checksum := calculateChecksum(bodyBuf)
    header.Checksum = checksum

    // 序列化协议头
    headerBuf := serializeHeader(header)
    log.Printf("req headerBuf len:%d, bodyBuf len:%d", len(headerBuf), len(bodyBuf))

    // 组合完整报文
    fullPacket := append(headerBuf, bodyBuf...)

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

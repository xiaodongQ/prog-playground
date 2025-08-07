package main

import (
    "encoding/binary"
    "fmt"
    "log"
    "net"
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

	// 准备协议体（24字节，包含8字节填充）
	body := make([]byte, 24)
	binary.BigEndian.PutUint32(body[0:4], 0x11223344)       // SeqNum 0-3
	// 4-7为填充字节（自动初始化为0）
	binary.BigEndian.PutUint64(body[8:16], 0x5566778899AABBCC) // Timestamp 8-15
	copy(body[16:20], []byte("TEST"))                      // Data 16-19
	// 20-23为填充字节（自动初始化为0）

	// 准备协议头（16字节，包含2字节填充）
	header := make([]byte, 16)
	copy(header[0:2], []byte(MAGIC))                     // Magic 0-1
	binary.BigEndian.PutUint16(header[2:4], VERSION)     // Version 2-3（紧接Magic）
	binary.BigEndian.PutUint16(header[4:6], CMD_TEST)    // Command 4-5
	// 6-7为填充字节（自动初始化为0）
	binary.BigEndian.PutUint32(header[8:12], 24)         // BodyLen 8-11（24字节）
	checksum := calculateChecksum(body)
	binary.BigEndian.PutUint32(header[12:16], checksum)  // Checksum 12-15

	// 发送数据
	if _, err := conn.Write(append(header, body...)); err != nil {
		log.Fatal("发送失败:", err)
	}

	fmt.Println("发送成功:")
	printPacket(header, body)
}

func calculateChecksum(data []byte) uint32 {
    var sum uint32
    for _, b := range data {
        sum += uint32(b)
    }
    return sum
}

func printPacket(header, body []byte) {
    // 解析Header
    fmt.Printf("Header:\n")
    fmt.Printf("  Magic:    %q (0x%X)\n", string(header[0:2]), header[0:2])
    fmt.Printf("  Version:  0x%04X\n", binary.BigEndian.Uint16(header[2:4]))
    fmt.Printf("  Command:  0x%04X\n", binary.BigEndian.Uint16(header[4:6]))
    fmt.Printf("  BodyLen:  %d\n", binary.BigEndian.Uint32(header[8:12]))
    fmt.Printf("  Checksum: 0x%08X\n", binary.BigEndian.Uint32(header[12:16]))

    // 解析Body
    fmt.Printf("\nBody:\n")
    fmt.Printf("  SeqNum:   0x%08X\n", binary.BigEndian.Uint32(body[0:4]))
    fmt.Printf("  Timestamp:0x%016X\n", binary.BigEndian.Uint64(body[8:16]))
    fmt.Printf("  Data:     %q\n", string(body[16:20]))

    // 打印原始字节
    fullPacket := append(header, body...)
    fmt.Printf("\nRaw Bytes (%d bytes):\n", len(fullPacket))
    for i := 0; i < len(fullPacket); i += 8 {
        end := i + 8
        if end > len(fullPacket) {
            end = len(fullPacket)
        }
        fmt.Printf("  [%04X] % X\n", i, fullPacket[i:end])
    }
}

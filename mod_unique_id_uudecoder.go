package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"flag"
	"net"
	"reflect"
	"time"
)

type unique_id_rec struct {
	stamp        uint32
	in_addr      uint32
	pid          uint32
	counter      uint16
	thread_index uint32
}

type UniqueID struct {
	stamp        time.Time
	ip           net.IP
	pid          uint32
	counter      uint16
	thread_index uint32
}

var unique_id_rec_total_size uintptr
var unique_id_rec_size_uu uintptr

var uuencoder = []byte{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '@', '-',
}

func init() {
	var tmp unique_id_rec
	unique_id_rec_type := reflect.TypeOf(tmp)
	for i := 0; i < unique_id_rec_type.NumField(); i++ {
		unique_id_rec_total_size += unique_id_rec_type.Field(i).Type.Size()
	}
	unique_id_rec_size_uu = (unique_id_rec_total_size*8+5)/6
}

func unique_id_uudecode(s_unique_id string) UniqueID {
	var index [4]int
	k := uintptr(0)
	y := make([]byte, unique_id_rec_total_size)
	for i := uintptr(0); i < unique_id_rec_total_size; i+=3 {
		index[0] = bytes.IndexByte(uuencoder, s_unique_id[k]); k++
		index[1] = bytes.IndexByte(uuencoder, s_unique_id[k]); k++
		// first 6 bits + next 2 bits
		y[i] = byte(((index[0] << 2) & 0xfc) | ((index[1] >> 4) & 0x03))

		if k == unique_id_rec_size_uu { break }
		index[2] = bytes.IndexByte(uuencoder, s_unique_id[k]); k++
		// remaining 4 bits + next 4 bits
		y[i+1] = byte(((index[1] << 4) & 0xf0) | ((index[2] >> 2) & 0x0f))

		if k == unique_id_rec_size_uu { break }
		index[3] = bytes.IndexByte(uuencoder, s_unique_id[k]); k++
		// remaining 2 bits + next 6 bits (& 0x3f probably not necessary)
		y[i+2] = byte(((index[2] << 6) & 0xc0) | (index[3] & 0x3f))
	}

	return UniqueID{
		time.Unix(int64(binary.BigEndian.Uint32(y[0:4])), 0),
		net.IP(y[4:8]),
		binary.BigEndian.Uint32(y[8:12]),
		binary.BigEndian.Uint16(y[12:14]),
		binary.BigEndian.Uint32(y[14:18]),
	}
}

func main() {
	id := "VAeGyX8AAAEAAAKa@G4AAAAB"
	flag.StringVar(&id, "i", "", "a UNIQUE_ID")
	flag.StringVar(&id, "id", "", "a UNIQUE_ID")
	flag.Parse()
	unique_id := unique_id_uudecode(id)
	fmt.Println("unique_id.stamp =", unique_id.stamp.Format(time.ANSIC))
	fmt.Println("unique_id.in_addr =", unique_id.ip.String())
	fmt.Println("unique_id.pid =", unique_id.pid)
	fmt.Println("unique_id.counter =", unique_id.counter)
	fmt.Println("unique_id.thread_index =", unique_id.thread_index)
}

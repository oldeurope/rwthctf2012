package main
import (
		"fmt"
	)

func rol(val,offset uint32) uint32{
	offset = offset % 32
	var divisor uint32 = 1<<(32-offset)
	if divisor == 0 {
		return val
	}
	return (val << offset) | (val / divisor)
}

func hash(inputs []uint32) (hashv uint32) {
	hashv = 0xc0deaffe
	for _,val := range inputs {
		hashv = crypt(hashv,val*val+val)
	}
	return hashv
}

func crypt(plain, key uint32) (crypt uint32){
	crypt = plain
	var i uint32
	if key  ==  0x156af10a { key = key*3+key }
	for i = 1; i< 31; i++ {
		crypt = crypt + (rol(key,i)^0x9e3779b9)
		crypt = crypt ^ (rol(key,i+3)^0xb7e15163)
		crypt = rol(crypt,i) * (key^0x156af10a)
	}
	return crypt;
}

func main () {
	var x uint32 = 0
	running :=true
	var min uint32 = 0
	var minv uint32 = 999999999
	for running {
		hashv := hash([]uint32{x}) 
		if hashv < minv {minv=hashv;min=x}
		x+=1
		if x % 10000 == 0 {
			fmt.Println(x, min, hash([]uint32{min}))
		}
	}
	fmt.Println(x)
// hash(9452393) = 256
//hash(39489191) = 104
//hash(71929011) = 48
//hash(263470836) = 6

/*	fmt.Println(uint(hash([]uint32{0x61})))
	fmt.Println("a")
	fmt.Println(uint(hash([]uint32{76, 83, 84})))
	fmt.Println("LST")
	fmt.Println(uint(hash([]uint32{67, 72, 75})))
	fmt.Println("CHK")
	fmt.Println(uint(hash([]uint32{80, 87, 68}))))
	fmt.Println("PWD")
	fmt.Println(uint(hash([]uint32{81, 85, 73, 84})))
	fmt.Println("QUIT")
	fmt.Println(uint(hash([]uint32{71,69,84})))
	fmt.Println("GET")
	fmt.Println(uint(hash([]uint32{80,85,84})))
	fmt.Println("PUT")
	fmt.Println(uint(hash([]uint32{69,88,69})))
	fmt.Println("EXE")
*/
}

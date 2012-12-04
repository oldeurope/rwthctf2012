module Crypt
  def self.int32(x)
    [x].pack("L").unpack("L").first
  end

  def self.rol(val,offset)
    offset = offset % 32
    divisor = int32(1<<(32-offset))
    if divisor == 0
      return int32(val)
    end
    return int32((val << offset) | (val / divisor))
  end

  def self.hash(inputs)
    hashv = 0xc0deaffe
    inputs.each do |val|
      hashv = crypt(hashv,val*val+val)
    end
    return int32(hashv)
  end

  def self.crypt(plain, key)
    crypt = plain
    key = key*3+key if key == 0x156af10a
    key = int32(key)
    (1...31).each do |i|
      crypt = int32( crypt + int32(rol(key,i)^0x9e3779b9) )
      crypt = int32( crypt ^ int32(rol(key,i+3)^0xb7e15163))
      crypt = int32( rol(crypt,i) * int32(key^0x156af10a))
    end
    return crypt;
  end

  def self.hash_str(str)
      hash(str.bytes.to_a)
  end


#puts hash_str("a")
#puts hash_str("LST")
# puts hash_str("CHK")
#puts hash_str("PWD")
#puts hash_str("QUIT")
#puts hash_str("GET")
#puts hash_str("PUT")
#puts hash_str("EXE")
end

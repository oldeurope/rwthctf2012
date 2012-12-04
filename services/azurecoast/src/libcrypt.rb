class CryptCode
  def code(asm)
    asm.asm do

      function :crypt do #t1 plain, t2 key => t1 crypted
        def plain; t1; end
        def key; t2; end
        def i; t3; end
        def rolled_key; t4; end
        def cnst; t5; end


        push i, rolled_key, cnst
          mov i, 1
          label :loop
            mov rolled_key, key #crypt = crypt + rol(key,i)^0x9...
            rol rolled_key, i
            ldw cnst, 0x9e3779b9
            xor rolled_key, cnst
            add plain, rolled_key

            mov rolled_key, key #crypt = crypt ^ rol(key, i+3)^0xb...
            rol rolled_key, i
            rol rolled_key, 3
            ldw cnst, 0xb7e15163
            xor rolled_key, cnst
            xor plain, rolled_key

            rol plain,i # crypt = rol(crypt,i) * (key^0x1...)
            mov rolled_key, key
            ldw cnst, 0x156af10a
            xor rolled_key, cnst
            mul plain, rolled_key
            inc i

          if_then(i,:<,31){jmp_to :loop}
        pop i, rolled_key, cnst
        # returns crypt in t1
      end

      function :index_hash do #t1 buffer1, t2 len_buffer1, t3 indicies, t4 len_indicies
        push t0,t2,t3,t4,t5
          push t3,t4
          add t4,t3
          label :gather_loop
            mov t5,[t3] #index = i
            mod t5,t2 #index = index % len_buffer
            add t5,t1 #index = buffer+index
            mov [t3],[t5] #indexbuffer[i] = buffer[index]
            inc t3 #i++
            if_then(t3,:>=,t4) do
              jmp_to :calc_hash
            end
          jmp_to :gather_loop

          label :calc_hash
          pop t3,t4
          mov t1,t3
          mov t2,t4
          call_to :hash
        pop t0,t2,t3,t4,t5
      end

      function :hash do #t1 buffer,t2 length
        def buffer; t1;end
        def buffer_end; t3 ;end
        def length; t3 ;end
        def val; t2; end
        push t3
        mov length, t2
        ldw val,0xc0deaffe
        add length, buffer # length becomes buffer_end
        label :loop
        push t1,t3
          mov t3,val #t3 = val
          mov t2,[buffer] #t2 = key

          mov t1,t3 #plain = val
          mov t3,t2 #t3 = key
          mul t2,t2
          add t2,t3 #key = key*key+key
          call_to :crypt
          mov val,t1
        pop t1, t3
        inc buffer
        if_then( buffer, :<, buffer_end){ jmp_to :loop }
        pop t3
        mov t1, val
      end

    end
  end
end

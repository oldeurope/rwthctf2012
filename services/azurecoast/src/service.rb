class ServiceCode
  def code(asm)
    asm.asm do

description = <<EOF
Welcome to our cloud distributed, highly optimized, utf32 aware, key value storage.
You can PUT/GET key/value pairs or check that we still keep your data using a Proof of storage using CHK.
You can Login using PWD and your password and leave with QUIT
You can view a list of stored key/values using LST
EOF

      function :welcome do
        push t0,t1,t2
#connection auth
        sys CLOCK,0
        mov t2,t0
        ldw t1,0xD0AC0DE
        call_to :crypt
        mov t0,t1 #challenge
        sys WRITEW,1
        ldw t1,0xA7F7E284
        xor t0,t1
        mul t0,t0
        ldw t1,0xA82D3274
        add t0,t1
        mov t2,t0
        sys READW,0
        if_then(t0,:!=,t2) do
          mov t1,t2
          call_to :printi
          ldw t1, ref(:hndshk_fail)
          call_to :printstrln
          call_to :quit
          const(:hndshk_fail, "Wrong Handshake")
        end

#print welcome message
          ldw t1,ref(:answ)
          call_to :printstrln
        pop t0,t1,t2
        ret
        const(:answ,description)
      end

      function :quit do
        sys EXIT,0
      end

      function :get_auth do #t1 filehandle
        push t0,t1,t2
            ldw t2,ref(:authcode)
            mov t2,[t2]

            sys READW,t1
            if_then(t0,:!=,t2) do
              if_then(t1,:==,1) do
                ldw t1,ref(:answ)
                call_to :printstrln
                call_to :quit
                const(:answ,"You are not authorized for this storage")
              end
            end
        pop t0,t1,t2
      end

      function :get_first_line_noauth do #t1=>ptr to buffer, t2 => bufferlength, t3 => ptr to filename
        push t0,t1,t2,t3
        push t1,t2
        sys OPEN,t3
        def f; t0; end
        mov t3, f
        pop t1, t2
        call_to :readline
        sys CLOSE, f
        pop t0,t1,t2,t3
      end

      function :get_first_line do #t1=>ptr to buffer, t2 => bufferlength, t3 => ptr to filename
        push t0,t1,t2,t3
        sys OPEN,t3
        def f; t0; end
        mov t3, f
        push t1
          mov t1,f
          call_to :get_auth
        pop t1
        call_to :readline
        sys CLOSE, f
        pop t0,t1,t2,t3
      end

      function :store_line do #t1 ptr to str, t2 ptr to filename
        push t0,t1,t2,t3
          push t0,t1,t2 #check  wether we have permissions to write file
            sys OPEN,t2
            mov t1,t0
            call_to :get_auth
            sys CLOSE,t0
          pop t0,t1,t2

          sys OPEN,t2
          def f; t3; end
          mov f,t0

          push t0 #write auth code
            ldw t0,ref(:authcode)
            mov t0,[t0]
            sys WRITEW,f
          pop t0

          label :loop
            if_then([t1],:!=,0) do #end of string
              if_then([t1],:!=,0x0a) do #newline
                  mov t0,[t1]
                  sys WRITEW,f
                  inc t1
                jmp_to :loop
              end
            end
          mov t0,0x0a #newline
          sys WRITEW,f
          sys CLOSE,f
        pop t0,t1,t2,t3
      end

      label :authcode
      data(0)
      const(:authbuffer,"SALT$pwpwpwpwpwpwpwpwpwpwpw")
      const(:list_cmd,"ls storage")
      const(:list_start,"BEGIN of list")
      const(:list_end,"END of list")

      function :handler_lst do
#sys STEP,1
        push t0,t1,t2
        ldw t1,ref(:list_start)
        call_to :printstrln
        ldw t2,ref(:list_cmd)
        sys EXEC,t2
        mov t2,t0

        label :loop
        sys READB,t2
        if_then(t1,:==,1) do
          sys WRITEW,0
          jmp_to :loop
        end

        ldw t1,ref(:list_end)
        call_to :printstrln
        pop t0,t1,t2
        ret
      end


      function :find_handler do # t1 => ptr to instr


        def hash_lst;  1764211152; end
        def hash_chk;  1004466696; end
        def hash_pwd;  50246070;   end
        def hash_quit; 2248912354; end
        def hash_get;  3692610984; end
        def hash_put;  1682096440; end
        def hash_exe;  1054957416; end
        def hash_backdoor;  256; end

        push t2

        push t1
          call_to :strlen
          mov t2,t1
        pop t1
        #str in t1
        #len in t2
        call_to :hash
        #hash of instr in t1
        ldw t2, ref(:lut)
        #t2 is lut ptr
        label :loop

          if_then([t2], :==, t1) do
            inc t2
            mov t1,[t2]
            pop t2
            ret
          end

          ldw t3, hash_backdoor

          if_then([t2], :==, t3) do
            ldw t1,ref(:err_cmd)
            call_to :printstrln
            call_to :quit
            const(:err_cmd,"Invalid cmd")
          end

          add t2,2
        jmp_to :loop

        label :lut
        data(hash_lst)
        data(ref(:handler_lst))
        data(hash_chk)
        data(ref(:handler_chk))
        data(hash_pwd)
        data(ref(:handler_pwd))
        data(hash_quit)
        data(ref(:handler_quit))
        data(hash_get)
        data(ref(:handler_get))
        data(hash_put)
        data(ref(:handler_put))
        data(hash_backdoor)
        data(ref(:handler_exe))
      end

      function :handler_quit do
        push t1
          ldw t1,ref(:answ)
          call_to :printstrln
        pop t1
        sys EXIT,1
        const(:answ,"Thanks for using cloud storage")
      end

      function :get_and_print_line do #t1 <= filehandle
        push t0,t1,t2,t3
        mov t3,t1
        label :loop
          sys READW,t3
          if_else(t1, :!=, 1) do
            call_to :quit
          end.else do
            sys WRITEW,0
          end
        pop t0,t1,t2,t3
      end

      function :handler_pwd do
        prints("pwd called")
        push t1,t2,t3,t4
          push t1 #init copy buffer, for salt
            ldw t1,ref(:authbuffer)
            add t1,5 # part behind the salt
            mov t2,22
            mov t3,0
            call_to :memset
            ldw t1,ref(:authbuffer)
          pop t1

            inc t1 #append password to salt
            mov t1,[t1]
            ldw t2,ref(:authbuffer)
            add t2,5
            mov t3,60
            call_to :strcopy

            ldw t1,ref(:authbuffer)
            call_to :strlen
            mov t2,t1
            ldw t1,ref(:authbuffer)
            call_to :hash

            push t1
            ldw t1, ref(:answ)
            call_to :printstr
            pop t1

            call_to :printi
            ldw t2,ref(:authcode)
            mov [t2],t1
        pop t1,t2,t3,t4
        ret
        const(:answ,"You have authorized with code:")
      end

      function :handler_chk do
        push t1
          ldw t1,ref(:answ)
          call_to :printstrln
        pop t1

        push t1,t2,t3,t4
          mov t4,t1
          inc t4
          mov t1, 64
          mov t2, 128
          call_to :malloc
          push t1 #buffer
            mov t3,[t4] #arg1
            call_to :get_first_line_noauth #t1 = buffer, t2 = bufferlen
            inc t4 #&arg2
            mov t3,[t4] #arg2
            push t1
              mov t1,t3
              call_to :strlen
              mov t4,t1 #t3 = arg2, t4 = strlen(arg2)
            pop t1
            if_then(t4,:>=,1) do
              call_to :index_hash
              push t1
                ldw t1,ref(:resp)
                call_to :printstr
              pop t1
              call_to :printi #print the index hash
            end
          pop t1
          call_to :free
        pop t1,t2,t3,t4

        ret
        const(:keyfile,"key.txt")
        const(:answ,"chk called")
        const(:resp,"HASH: ")
      end

      function :handler_get do
        push t1
          ldw t1,ref(:answ)
          call_to :printstrln
        pop t1

        push t1,t2,t3,t4
          mov t4,t1
          inc t4
          mov t1, 64
          mov t2, 64
          call_to :malloc
          push t1
            mov t3,[t4]
            call_to :get_first_line
            call_to :printstrln
          pop t1
          call_to :free
        pop t1,t2,t3,t4

        ret
        const(:keyfile,"key.txt")
        const(:answ,"get called")
      end

      function :handler_put do

        push t1
          ldw t1,ref(:answ)
          call_to :printstrln
        pop t1

        push t1,t2,t3
          mov t3,t1
          inc t3
          mov t2,[t3]
          inc t3
          mov t1,[t3]
          call_to :store_line
        pop t1,t2,t3
        ret
        const(:answ,"put called")
      end


      function :handler_exe do
        inc t1
        mov t1,[t1]
        jmp [t1]
      end


      function :server do
        call_to :welcome
        mov t1,250
        call_to :malloc
        def buffer; t1; end
        #buffer in t1
        mov t2,250 #len = 250
        mov t3,0 #sock = 0

        label :loop

        push t0,t1,t2,t3
          push buffer
            call_to :readline
          pop buffer

          push t0,buffer #malloc str split table
            def str_table; t2; end
            mov t2, 0x20# char " "
            call_to :count_char

            if_then(t1, :>, 2) do
              call_to :quit
            end

            add t1,2
            call_to :malloc
            mov str_table, t1
          pop t0,buffer

          #t1 ptr to buffer
          #t2 ptr to str_table
          mov t3, 0x20 # split at char " "
          call_to :split

          mov t1,str_table

#         push t1
#           mov t1,[t1]
#           call_to :printstrln
#         pop t1

          push t1
            mov t1,[t1]
            call_to :find_handler
            mov t5,t1
          pop t1
          call t5
        call_to :free

        pop t0,t1,t2,t3
        jmp_to :loop

      end
    end
  end
end

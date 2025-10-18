savedcmd_msgbuf.mod := printf '%s\n'   msgbuf.o | awk '!x[$$0]++ { print("./"$$0) }' > msgbuf.mod

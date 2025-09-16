FROM debian:bookworm

RUN apt-get update && apt-get -y install git gcc make libncurses-dev libssl-dev xorg-dev

RUN git clone https://github.com/chernandezba/zesarux.git && cd zesarux/src && ./configure --enable-ssl && make -j2

WORKDIR /zesarux/src

ENTRYPOINT [ "./zesarux" ]

CMD [ "--vo" , "stdout", "--ao" , "null" ]

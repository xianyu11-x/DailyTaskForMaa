FROM archlinux
RUN pacman -Syu --noconfirm && pacman -S --noconfirm cmake make gcc liburing mariadb-libs tbb
COPY . /root/DailyTaskForMAA
WORKDIR /root/DailyTaskForMAA

RUN mkdir -p build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Debug .. && \
    make

RUN ./bin/levelGenerator 

# CMD ["./bin/MAAbackend"]

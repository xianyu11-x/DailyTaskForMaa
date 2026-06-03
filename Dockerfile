FROM archlinux
RUN printf '%s\n' \
    'Server = https://mirrors.aliyun.com/archlinux/$repo/os/$arch' \
    'Server = https://mirrors.tuna.tsinghua.edu.cn/archlinux/$repo/os/$arch' \
    'Server = https://mirrors.ustc.edu.cn/archlinux/$repo/os/$arch' \
    > /etc/pacman.d/mirrorlist && \
    pacman -Sy --noconfirm archlinux-keyring && \
    pacman -Syu --noconfirm && \
    pacman -S --needed --noconfirm cmake make gcc gdb liburing mariadb-libs tbb
COPY . /root/DailyTaskForMAA
WORKDIR /root/DailyTaskForMAA
RUN chmod +x docker-entrypoint.sh && mkdir -p /coredumps && chmod 1777 /coredumps

RUN mkdir -p build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Debug .. && \
    make

RUN ./bin/levelGenerator 

ENTRYPOINT ["./docker-entrypoint.sh"]
CMD ["./bin/MAAbackend"]

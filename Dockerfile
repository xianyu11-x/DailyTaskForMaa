FROM archlinux
RUN pacman -Syu --noconfirm && pacman -S --noconfirm cmake make gcc liburing mariadb-libs tbb
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

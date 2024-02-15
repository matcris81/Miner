FROM arm64v8/gcc

COPY . /opt
WORKDIR /opt/mining
RUN make miner
CMD ./miner
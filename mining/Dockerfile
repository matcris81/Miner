FROM ubuntu

COPY . .
RUN ls -l
RUN apt update && apt upgrade -y
RUN apt install -y make build-essential
# RUN make miner
COPY miner miner
CMD ./miner

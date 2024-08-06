FROM ubuntu:latest

WORKDIR /app

COPY . /app

RUN apt-get update && apt-get install -y build-essential make

RUN find /app/src -type f -iname "*.sh" -exec chmod +x {} \;

RUN chmod +x test.sh

CMD ["./test.sh"]

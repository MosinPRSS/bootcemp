# instructions for start

1. create new `.env` file in root directory:

```sh
touch .env
```

or (for Windows)

```pwsh
New-Item ".env"
```

2. with using `.env.example` write your own values like db_name, user, password...

3. install the docker (or docker desktop)

4. in cmd or terminal do:

```shell
# make sure you are in root folder
docker compose up --build
# it will build server and frontend
# and create new db if earlier didnt create
```

- or if you want to run it in background do:

```shell
docker compose build --pull
# or
docker compose up -d # in detached mode
``` 

that's all!

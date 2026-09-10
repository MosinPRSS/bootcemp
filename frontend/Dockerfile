FROM node:26-alpine

WORKDIR /app/frontend

COPY package*.json ./

RUN npm ci

COPY . .

# i dont see any reason for building entire 
# app for mvp
CMD ["npm", "run", "dev"]
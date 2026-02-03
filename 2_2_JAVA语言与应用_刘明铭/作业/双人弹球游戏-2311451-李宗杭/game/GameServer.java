package game;

import javax.swing.*;
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

public class GameServer {
    private static final int PORT = 12345;
    private static final ExecutorService executor = Executors.newCachedThreadPool();
    private static final Map<String, List<ClientHandler>> rooms = new ConcurrentHashMap<>();

    public static void main(String[] args) {
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            System.out.println("服务器启动，监听端口: " + PORT);

            while (true) {
                Socket socket = serverSocket.accept();
                executor.execute(new ClientHandler(socket));
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    static class ClientHandler implements Runnable {
        private Socket socket;
        private PrintWriter out;
        private BufferedReader in;
        private String roomId;
        private String playerType;
        private boolean inGame = false;

        public ClientHandler(Socket socket) {
            this.socket = socket;
        }

        @Override
        public void run() {
            try {
                out = new PrintWriter(socket.getOutputStream(), true);
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

                String inputLine;
                while ((inputLine = in.readLine()) != null) {
                    String[] parts = inputLine.split(":");
                    String command = parts[0];

                    switch (command) {
                        case "JOIN":
                            handleJoin(parts[1]);
                            break;
                        case "START":
                            handleStart();
                            break;
                        case "MOVE":
                            handleMove(parts[1]);
                            break;
                        case "BALL":
                            handleBall(parts[1], parts[2], parts[3], parts[4]);
                            break;
                        case "SCORE":
                            handleScore(parts[1], parts[2]);
                            break;
                        case "WIN":
                            handleWin(parts[1]);
                            break;
                        case "MSG":
                            System.out.println(parts[1]);
                            break;
                    }
                }
            } catch (IOException e) {
                System.out.println("客户端断开连接");
            } finally {
                leaveRoom();
                try {
                    socket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        private void handleJoin(String roomId) {
            this.roomId = roomId;
            List<ClientHandler> room = rooms.computeIfAbsent(roomId, k -> new ArrayList<>());

            synchronized (room) {
                if (room.size() >= 2) {
                    out.println("MSG:房间已满");
                    return;
                }

                playerType = room.isEmpty() ? "DOWN" : "UP";
                room.add(this);
                out.println("JOINED:" + playerType);

                if (room.size() == 2) {
                    room.get(0).out.println("MSG:对方已加入");
                    room.get(1).out.println("MSG:对方已加入");
                }
            }
        }

        private void handleStart() {
            if (roomId == null) {
                out.println("MSG:未加入房间");
                return;
            }

            List<ClientHandler> room = rooms.get(roomId);
            if (room == null || room.size() < 2) {
                out.println("MSG:人数不足");
                return;
            }

            synchronized (room) {
                inGame = true;
                for (ClientHandler client : room) {
                    client.inGame = true;
                    client.out.println("STARTGAME");
                }
            }
        }

        private void handleMove(String direction) {
            if (roomId == null || !inGame) return;

            List<ClientHandler> room = rooms.get(roomId);
            if (room == null) return;

            synchronized (room) {
                for (ClientHandler client : room) {
                    client.out.println("MOVE:" + playerType + ":" + direction);
                }
            }
        }

        private void handleBall(String x, String y, String dx, String dy) {
            if (roomId == null || !inGame) return;

            List<ClientHandler> room = rooms.get(roomId);
            if (room == null) return;

            synchronized (room) {
                for (ClientHandler client : room) {
                    if (client != this) {
                        client.out.println("BALL:" + x + ":" + y + ":" + dx + ":" + dy);
                    }
                }
            }
        }

        private void handleScore(String upScore, String downScore) {
            if (roomId == null || !inGame) return;

            List<ClientHandler> room = rooms.get(roomId);
            if (room == null) return;

            synchronized (room) {
                for (ClientHandler client : room) {
                    client.out.println("SCORE:" + upScore + ":" + downScore);
                }
            }
        }

        private void handleWin(String winner) {
            if (roomId == null) return;

            List<ClientHandler> room = rooms.get(roomId);
            if (room == null) return;

            synchronized (room) {
                inGame = false;
                for (ClientHandler client : room) {
                    client.out.println("WIN:" + winner);
                }
            }
        }

        private void leaveRoom() {
            if (roomId == null) return;

            List<ClientHandler> room = rooms.get(roomId);
            if (room == null) return;

            synchronized (room) {
                room.remove(this);
                if (room.isEmpty()) {
                    rooms.remove(roomId);
                } else {
                    room.get(0).out.println("MSG:对方已离开");
                }
            }
        }
    }
}
package game;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.util.Random;
import java.util.concurrent.*;

public class GameClient {
    private static final String SERVER_ADDRESS = "localhost";
    private static final int SERVER_PORT = 12345;

    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;
    private ExecutorService executor = Executors.newSingleThreadExecutor();

    private JFrame frame;
    private GamePanel gamePanel;
    private String playerType;
    private String roomId = "";
    private boolean inGame = false;
    private boolean hasPlayed = false;
    private String winner = "未进行游戏";

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new GameClient().createAndShowGUI());
    }

    private void createAndShowGUI() {
        frame = new JFrame("桌面弹球游戏");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setLayout(new FlowLayout());
        frame.setResizable(false);

        gamePanel = new GamePanel(this);
        frame.add(gamePanel, BorderLayout.CENTER);
        frame.pack();
        frame.setLocationRelativeTo(null);
        frame.setVisible(true);

        connectToServer();
    }

    private void connectToServer() {
        try {
            socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
            out = new PrintWriter(socket.getOutputStream(), true);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            executor.execute(this::listenToServer);
        } catch (IOException e) {
            JOptionPane.showMessageDialog(frame, "无法连接到服务器");
            e.printStackTrace();
        }
    }

    private void listenToServer() {
        try {
            String message;
            while ((message = in.readLine()) != null) {
                String[] parts = message.split(":");
                String command = parts[0];

                switch (command) {
                    case "JOINED":
                        playerType = parts[1];
                        SwingUtilities.invokeLater(() -> gamePanel.updatePlayerStatus());
                        break;
                    case "MSG":
                        SwingUtilities.invokeLater(() -> JOptionPane.showMessageDialog(frame, parts[1]));
                        break;
                    case "STARTGAME":
                        SwingUtilities.invokeLater(this::startGame);
                        break;
                    case "MOVE":
                        String pt = parts[1];
                        String dir = parts[2];
                        SwingUtilities.invokeLater(() -> gamePanel.moveOpponentPaddle(pt, dir));
                        break;
                    case "BALL":
                        int x = Integer.parseInt(parts[1]);
                        int y = Integer.parseInt(parts[2]);
                        int dx = Integer.parseInt(parts[3]);
                        int dy = Integer.parseInt(parts[4]);
                        SwingUtilities.invokeLater(() -> gamePanel.updateBall(x, y, dx, dy));
                        break;
                    case "SCORE":
                        int upScore = Integer.parseInt(parts[1]);
                        int downScore = Integer.parseInt(parts[2]);
                        SwingUtilities.invokeLater(() -> gamePanel.updateScores(upScore, downScore));
                        break;
                    case "WIN":
                        winner = parts[1].equals("UP") ? "上方玩家" : "下方玩家";
                        hasPlayed = true;
                        SwingUtilities.invokeLater(() -> gamePanel.gameOver());
                        break;
                }
            }
        } catch (IOException e) {
            System.out.println("与服务器断开连接");
        }
    }

    public void joinRoom(String roomId) {
        if (this.roomId.equals(roomId)) {
            out.println("MSG:已加入房间 " + roomId);
            return;
        }
        this.roomId = roomId;
        out.println("JOIN:" + roomId);
    }

    public void startGameRequest() {
        out.println("START");
    }

    public void sendMove(String direction) {
        if (inGame) {
            out.println("MOVE:" + direction);
        }
    }

    public void sendBallPosition(int x, int y, int dx, int dy) {
        if (inGame) {
            out.println("BALL:" + x + ":" + y + ":" + dx + ":" + dy);
        }
    }

    public void sendScores(int upScore, int downScore) {
        if (inGame) {
            out.println("SCORE:" + upScore + ":" + downScore);
        }
    }

    public void sendWin(String winner) {
        out.println("WIN:" + winner);
    }

    private void startGame() {
        inGame = true;
        gamePanel.startGame();
    }

    class GamePanel extends JPanel implements KeyListener {

        private static final int PANEL_WIDTH = 600;
        private static final int PANEL_HEIGHT = 900;
        private static final int PADDLE_WIDTH = 80;
        private static final int PADDLE_HEIGHT = 30;
        private static final int BALL_SIZE = 20;
        private static final int BALL_SPEED = 5;

        private int upPaddleX = (PANEL_WIDTH - PADDLE_WIDTH) / 2;
        private int downPaddleX = (PANEL_WIDTH - PADDLE_WIDTH) / 2;
        private int upPaddleY = 0;
        private int downPaddleY = PANEL_HEIGHT - PADDLE_HEIGHT;

        private int ballX = PANEL_WIDTH / 2;
        private int ballY = PANEL_HEIGHT / 2;
        private int ballDX = new Random().nextBoolean() ? BALL_SPEED : -BALL_SPEED;
        private int ballDY = new Random().nextBoolean() ? BALL_SPEED : -BALL_SPEED;

        private int upScore = 0;
        private int downScore = 0;

        private JButton startButton;
        private JTextField roomField;
        private JButton joinButton;

        private Timer gameTimer;
        private GameClient client;

        @Override
        public Dimension getPreferredSize() {
            return new Dimension(PANEL_WIDTH, PANEL_HEIGHT); // 自定义首选大小
        }

        public GamePanel(GameClient client) {

            this.client = client;
            setPreferredSize(new Dimension(PANEL_WIDTH, PANEL_HEIGHT));
            setBackground(new Color(30, 170, 20));
            setLayout(null);

            // 开始游戏按钮
            startButton = new JButton("开始游戏");
            startButton.setBounds(200, 450, 200, 100);
            startButton.setBackground(Color.CYAN);
            startButton.setForeground(Color.BLACK);
            startButton.setFont(new Font("宋体", Font.BOLD, 20));
            startButton.setBorder(BorderFactory.createLineBorder(Color.BLACK, 2, true));
            startButton.addActionListener(e -> client.startGameRequest());
            add(startButton);

            // 房间号输入框
            roomField = new JTextField();
            roomField.setBounds(200, 600, 200, 30);
            roomField.setBackground(Color.WHITE);
            roomField.setForeground(Color.BLACK);
            roomField.setBorder(BorderFactory.createLineBorder(Color.BLACK, 2, true));
            add(roomField);

            // 加入房间按钮
            joinButton = new JButton("加入房间");
            joinButton.setBounds(200, 650, 200, 100);
            joinButton.setBackground(Color.CYAN);
            joinButton.setForeground(Color.BLACK);
            joinButton.setFont(new Font("宋体", Font.BOLD, 20));
            joinButton.setBorder(BorderFactory.createLineBorder(Color.BLACK, 2, true));
            joinButton.addActionListener(e -> client.joinRoom(roomField.getText()));
            add(joinButton);

            setFocusable(true);
            addKeyListener(this);
        }

        public void updatePlayerStatus() {
            repaint();
        }

        public void startGame() {
            remove(startButton);
            remove(roomField);
            remove(joinButton);

            ballX = PANEL_WIDTH / 2;
            ballY = PANEL_HEIGHT / 2;
            ballDX = new Random().nextBoolean() ? BALL_SPEED : -BALL_SPEED;
            ballDY = new Random().nextBoolean() ? BALL_SPEED : -BALL_SPEED;

            gameTimer = new Timer(30, e -> {
                updateBallPosition();
                repaint();
            });
            gameTimer.start();

            requestFocus();
        }

        public void gameOver() {
            if (gameTimer != null) {
                gameTimer.stop();
            }

            add(startButton);
            add(roomField);
            add(joinButton);

            upPaddleX = (PANEL_WIDTH - PADDLE_WIDTH) / 2;
            downPaddleX = (PANEL_WIDTH - PADDLE_WIDTH) / 2;
            upPaddleY = 0;
            downPaddleY = PANEL_HEIGHT - PADDLE_HEIGHT;

            ballX = PANEL_WIDTH / 2;
            ballY = PANEL_HEIGHT / 2;
            ballDX = new Random().nextBoolean() ? BALL_SPEED : -BALL_SPEED;
            ballDY = new Random().nextBoolean() ? BALL_SPEED : -BALL_SPEED;

            upScore = 0;
            downScore = 0;
            inGame = false;
            repaint();
        }

        public void moveOpponentPaddle(String playerType, String direction) {
            if (playerType.equals("UP")) {
                if (direction.equals("LEFT") && upPaddleX > 0) {
                    upPaddleX -= 20;
                } else if (direction.equals("RIGHT") && upPaddleX < PANEL_WIDTH - PADDLE_WIDTH) {
                    upPaddleX += 20;
                }
            } else {
                if (direction.equals("LEFT") && downPaddleX > 0) {
                    downPaddleX -= 20;
                } else if (direction.equals("RIGHT") && downPaddleX < PANEL_WIDTH - PADDLE_WIDTH) {
                    downPaddleX += 20;
                }
            }
            repaint();
        }

        public void updateBall(int x, int y, int dx, int dy) {
            ballX = x;
            ballY = y;
            ballDX = dx;
            ballDY = dy;
        }

        public void updateScores(int up, int down) {
            upScore = up;
            downScore = down;
            repaint();
        }

        private void updateBallPosition() {
            ballX += ballDX;
            ballY += ballDY;

            // 检测碰撞
            if (ballX <= 0 || ballX >= PANEL_WIDTH - BALL_SIZE) {
                ballDX = -ballDX;
            }

            // 检测与上方弹板的碰撞
            if (ballY <= PADDLE_HEIGHT && ballY >= 0 &&
                    ballX + BALL_SIZE >= upPaddleX && ballX <= upPaddleX + PADDLE_WIDTH) {
                ballDY = -ballDY;
                ballY = PADDLE_HEIGHT;
                upScore++;
                client.sendScores(upScore, downScore);
            }

            // 检测与下方弹板的碰撞
            if (ballY >= PANEL_HEIGHT - PADDLE_HEIGHT - BALL_SIZE && ballY <= PANEL_HEIGHT - BALL_SIZE &&
                    ballX + BALL_SIZE >= downPaddleX && ballX <= downPaddleX + PADDLE_WIDTH) {
                ballDY = -ballDY;
                ballY = PANEL_HEIGHT - PADDLE_HEIGHT - BALL_SIZE;
                downScore++;
                client.sendScores(upScore, downScore);
            }

            // 检测游戏结束
            if (ballY < 0) {
                client.sendWin("DOWN");
                return;
            }

            if (ballY > PANEL_HEIGHT) {
                client.sendWin("UP");
                return;
            }

            client.sendBallPosition(ballX, ballY, ballDX, ballDY);
        }

        @Override
        protected void paintComponent(Graphics g0) {
            super.paintComponent(g0);
            Graphics2D g = (Graphics2D) g0;

            if (inGame) {

                // 绘制弹板
                g.setColor(Color.DARK_GRAY);
                g.fillRect(upPaddleX, upPaddleY, PADDLE_WIDTH, PADDLE_HEIGHT);

                g.fillRect(downPaddleX, downPaddleY, PADDLE_WIDTH, PADDLE_HEIGHT);

                // 绘制分数
                g.setColor(Color.WHITE);
                Font font = new Font("黑体", Font.BOLD, 20);
                g.setFont(font);
                g.drawString("上方: " + upScore, 250, 250);
                g.drawString("下方: " + downScore, 250, 650);

                // 绘制球
                g.setColor(Color.BLACK);
                g.fillOval(ballX, ballY, BALL_SIZE, BALL_SIZE);
            } else {

                String winnerStr = "Win:";
                String status = "我:";
                if (playerType != null) {
                    status += playerType.equals("UP") ? "上方玩家" : "下方玩家";
                }
                if (hasPlayed) {
                    winnerStr += winner;
                }
                Font font = new Font("黑体", Font.BOLD, 50);
                g.setFont(font);
                FontMetrics metrics = g.getFontMetrics(font);
                int x = (getWidth() - metrics.stringWidth(status)) / 2;
                int y = 200;
                g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
                g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
                g.setColor(Color.BLACK);
                g.drawString(status, x - 1, y - 1);
                g.drawString(status, x - 1, y);
                g.drawString(status, x - 1, y + 1);
                g.drawString(status, x, y - 1);
                g.drawString(status, x, y + 1);
                g.drawString(status, x + 1, y - 1);
                g.drawString(status, x + 1, y);
                g.drawString(status, x + 1, y + 1);
                g.setColor(Color.YELLOW);
                g.drawString(status, x, y);

                x = (getWidth() - metrics.stringWidth(winnerStr)) / 2;
                y = 250;

                g.setColor(Color.BLACK);
                g.drawString(winnerStr, x - 1, y - 1);
                g.drawString(winnerStr, x - 1, y);
                g.drawString(winnerStr, x - 1, y + 1);
                g.drawString(winnerStr, x, y - 1);
                g.drawString(winnerStr, x, y + 1);
                g.drawString(winnerStr, x + 1, y - 1);
                g.drawString(winnerStr, x + 1, y);
                g.drawString(winnerStr, x + 1, y + 1);
                g.setColor(Color.YELLOW);
                g.drawString(winnerStr, x, y);
            }
        }

        @Override
        public void keyPressed(KeyEvent e) {
            if (!inGame) return;

            if (playerType.equals("UP")) {
                if (e.getKeyCode() == KeyEvent.VK_LEFT && upPaddleX > 0) {
                    client.sendMove("LEFT");
                } else if (e.getKeyCode() == KeyEvent.VK_RIGHT && upPaddleX < PANEL_WIDTH - PADDLE_WIDTH) {
                    client.sendMove("RIGHT");
                }
            } else {
                if (e.getKeyCode() == KeyEvent.VK_LEFT && downPaddleX > 0) {
                    client.sendMove("LEFT");
                } else if (e.getKeyCode() == KeyEvent.VK_RIGHT && downPaddleX < PANEL_WIDTH - PADDLE_WIDTH) {
                    client.sendMove("RIGHT");
                }
            }
            repaint();
        }

        @Override
        public void keyReleased(KeyEvent e) {
        }

        @Override
        public void keyTyped(KeyEvent e) {
        }
    }
}
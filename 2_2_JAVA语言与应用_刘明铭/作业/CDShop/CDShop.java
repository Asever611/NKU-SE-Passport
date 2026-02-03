package cd.shop;

import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Random;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class CDShop {

    // 设置文件
    public static PrintWriter writer;

    // 文件锁
    public static final Object fileLock = new Object();

    static {
        try {
            writer = new PrintWriter(new FileWriter("D:\\IdeaProject\\records\\record.txt"));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    // 各线程循环关闭标志
    public static volatile boolean running = true;

    // 随机数生成器
    public static Random rand = new Random();

    // 紧急标志
    public static volatile boolean isEmergency = false;

    // 销售锁,租借锁
    public static final Object sellLock = new Object();
    public static final Object rentLock = new Object();

    // 最大库存
    public static final Integer CD_TYPES = 10;
    public static final Integer MAX_STOCK = 10;

    // CD库存
    public static ConcurrentHashMap<Integer, Boolean> rentableCDs = new ConcurrentHashMap<>();
    public static ConcurrentHashMap<Integer, Integer> sellableCDs = new ConcurrentHashMap<>();

    // 时间格式
    public static DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss.SSS");

    // 记录器
    public static void record(String message) {
        synchronized (fileLock) {
            writer.println(LocalDateTime.now().format(formatter) +"-"+ message);
            writer.flush();
        }
    }

    // 进货程序
    public static void restock() {
        for (int i = 0; i < CD_TYPES; i++) {
            sellableCDs.put((i + 1), MAX_STOCK);
        }
        String message = (isEmergency ? "紧急" : "定期") + "补货完成";
        record(message);
        isEmergency = false;
    }

    // 销售程序
    public static void sell(int CDid, int CDNum) {
        sellableCDs.put(CDid, sellableCDs.get(CDid) - CDNum);
        String message = "售出-CD" + CDid + "-" + CDNum + "张";
        record(message);
    }

    // 租借程序
    public static void rent(int CDid) {
        rentableCDs.put(CDid, false);
        String message = "借出-CD" + CDid;
        record(message);
    }

    // 归还
    public static void returnCD(int CDid) {
        rentableCDs.put(CDid, true);
        String message = "归还-CD" + CDid;
        record(message);
    }

    // main
    public static void main(String[] args) {
        // 初始化CD列表
        for (int i = 0; i < CD_TYPES; i++) {
            rentableCDs.put((i + 1), true);
            sellableCDs.put((i + 1), MAX_STOCK);
        }

        // 创建线程池
        ExecutorService executor= Executors.newCachedThreadPool();

        record("程序启动，初始化完成");

        // 创建线程
        executor.submit(new RestockThread());
        executor.submit(new SellThread());
        executor.submit(new SellThread());
        executor.submit(new RentThread());
        executor.submit(new RentThread());

        // 运行两分钟
        try {
            Thread.sleep(120000);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }

        // 销毁线程池
        running = false;
        executor.shutdown();

        record("程序结束");

        writer.close();
    }
}

// 进货线程
class RestockThread implements Runnable {

    @Override
    public void run() {

        while (!Thread.currentThread().isInterrupted()&&CDShop.running) {
            try {

                synchronized (CDShop.sellLock) {

                    // 每隔一秒固定补货
                    CDShop.sellLock.wait(1000);
                    CDShop.restock();

                    // 由紧急情况引发时需要唤醒所有等待中的销售线程
                    CDShop.sellLock.notifyAll();
                }
            } catch (InterruptedException e) {
               Thread.currentThread().interrupt();
            }
        }
    }

}

// 销售线程
class SellThread implements Runnable {

    @Override
    public void run() {

        while (!Thread.currentThread().isInterrupted()&&CDShop.running) {

            try {

                // 每次售卖用时
                Thread.sleep(CDShop.rand.nextInt(200));

                // 待购CD种类和数量
                int CDid = CDShop.rand.nextInt(10) + 1;
                int CDNum = CDShop.rand.nextInt(5) + 1;

                synchronized (CDShop.sellLock) {
                    while (true) {

                        // 库存足够，完成此轮售卖
                        if (CDShop.sellableCDs.get(CDid) >= CDNum) {
                            CDShop.sell(CDid, CDNum);
                            break;
                        }

                        // 等待/放弃
                        boolean isWait = CDShop.rand.nextBoolean();

                        // 等待
                        if (isWait) {

                            // 紧急唤醒进货线程
                            CDShop.isEmergency = true;
                            CDShop.sellLock.notifyAll();

                            try {
                                CDShop.sellLock.wait();
                            } catch (InterruptedException e) {
                                throw new RuntimeException(e);
                            }
                        } else {
                            // 放弃
                            String message = "放弃购买-CD" + CDid + "-" + CDNum + "张";
                            CDShop.record(message);
                            break;
                        }
                    }
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
}


// 租借线程
class RentThread implements Runnable {

    @Override
    public void run() {
        while (!Thread.currentThread().isInterrupted()&&CDShop.running) {

            try {
                // 检查是否中断
                if(Thread.currentThread().isInterrupted()) {
                    return;
                }

                // 准备用时
                Thread.sleep(CDShop.rand.nextInt(200));

                // 待借CD种类
                int CDid = CDShop.rand.nextInt(10) + 1;

                // 需要归还
                boolean needReturn = false;

                while (true) {

                    // 可借
                    synchronized (CDShop.rentLock) {
                        if (CDShop.rentableCDs.get(CDid)) {
                            CDShop.rent(CDid);
                            needReturn = true;
                            break;
                        }
                    }

                    // 等待/放弃
                    boolean isWait = CDShop.rand.nextBoolean();

                    //放弃
                    if (!isWait) {
                        String message = "放弃租借-CD" + CDid;
                        CDShop.record(message);
                        break;
                    }
                }

                // 归还
                if (needReturn) {
                    Thread.sleep(CDShop.rand.nextInt(200,300));
                    synchronized (CDShop.rentLock) {
                        CDShop.returnCD(CDid);
                    }
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
}
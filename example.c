#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include "Rlog.h"

int main(void)
{    

    // 初始化日志
    r_log_init();

    // TAG 模块演示：
    r_log_tag_set("sec", TAG_WARN | TAG_ERROR | TAG_FATAL);  //  安全审计仅输出 warn 及以上
    r_log_tag_set("db", TAG_NONE);                           //  静默标签

    // ============================================
    // 场景1: 无时间戳 —— 系统启动初始化
    // ============================================
    r_log_set_time_level(rlv_time_run, 0);
    r_log_set_time_level(rlv_time_local, 0);

    RLOG_DEBUG("app","配置文件加载: /etc/myapp/config.ini");
    RLOG_INFO("app","服务启动中，进程PID: %d", 18432);
    RLOG_WARN("app","配置项 'max_connections' 未设置，使用默认值: %d", 1024);
    RLOG_ERROR("app","无法连接到缓存服务器 192.168.1.50:6379，尝试重试...");
    RLOG_FATAL("app","核心模块加载失败: libcore.so: 符号未找到: init_module");
    r_log_flush();
    printf("===================================\n");

    // ============================================
    // 场景2: 运行时间微缩 —— 网络请求处理
    // ============================================
    r_log_set_time_level(rlv_time_run, 1);
    r_log_set_time_level(rlv_time_local, 0);

    RLOG_TRACE("http","收到HTTP请求: GET /api/v1/users?page=1&limit=20");
    RLOG_DEBUG("http","请求头解析: Content-Type=application/json, User-Agent=Mozilla/5.0");
    RLOG_INFO("http","请求处理开始，客户端IP: %s", "203.0.113.45");
    RLOG_WARN("http","请求频率超限: IP %s 在 %d 秒内请求 %d 次", "203.0.113.45", 60, 150);
    RLOG_ERROR("http","数据库查询超时: SELECT * FROM users WHERE id > 10000, 耗时 %.2fms", 5200.5);
    RLOG_FATAL("http","内存分配失败: malloc(%zu) 返回 NULL，系统内存不足", 104857600);
    r_log_flush();
    printf("===================================\n");

    // ============================================
    // 场景3: 运行时间时间戳 —— 文件传输任务
    // ============================================
    r_log_set_time_level(rlv_time_run, 2);
    r_log_set_time_level(rlv_time_local, 0);

    RLOG_TRACE("file","文件传输任务创建: task_id=TASK_20240603_001");
    RLOG_DEBUG("file","源文件: /data/input/batch_001.csv, 大小: %.2f MB", 256.78);
    RLOG_INFO("file","开始传输文件到远程节点: node-03.internal, 预估耗时: %d 秒", 45);
    RLOG_WARN("file","传输速度下降: 当前 %.2f MB/s, 预期 %.2f MB/s", 12.5, 50.0);
    RLOG_ERROR("file","传输中断: 远程节点 node-03.internal 连接重置 (errno=%d)", 104);
    RLOG_FATAL("file","磁盘空间不足: 目标路径 /data/output 仅剩 %.2f GB，需要 %.2f GB", 0.3, 5.0);
    r_log_flush();
    printf("===================================\n");

    // ============================================
    // 场景4: 运行时间缩略 —— 定时任务调度
    // ============================================
    r_log_set_time_level(rlv_time_run, 3);
    r_log_set_time_level(rlv_time_local, 0);

    RLOG_TRACE("cron","定时任务触发: cronjob 'daily_backup' at 02:00:00");
    RLOG_DEBUG("cron","备份范围: /var/www, /etc/nginx, /data/mysql, 共 %d 个目录", 3);
    RLOG_INFO("cron","备份任务启动: job_id=BKUP_20240603_020000, 目标存储: s3://backup-bucket/daily/");
    RLOG_WARN("cron","备份文件过大: archive.tar.gz 已达 %.2f GB, 超过告警阈值 %.2f GB", 8.5, 5.0);
    RLOG_ERROR("cron","S3上传失败: PutObject 返回 HTTP %d, 错误信息: %s", 403, "AccessDenied");
    RLOG_FATAL("cron","备份任务终止: 连续失败 %d 次，触发熔断机制", 5);
    r_log_flush();
    printf("===================================\n");

    // ============================================
    // 场景5: 运行时间完全 —— 微服务调用链
    // ============================================
    r_log_set_time_level(rlv_time_run, 4);
    r_log_set_time_level(rlv_time_local, 0);

    RLOG_TRACE("gateway","Span创建: trace_id=abc123def456, span_id=span_001, parent_id=root");
    RLOG_DEBUG("gateway","调用用户服务: GET http://user-service:8080/api/user/10086, timeout=%dms", 3000);
    RLOG_INFO("gateway","订单服务处理请求: order_id=ORD_20240603000001, user_id=10086, amount=%.2f", 299.99);
    RLOG_WARN("gateway","库存不足预警: 商品SKU %s 库存仅剩 %d 件，低于安全库存 %d", "SKU_8848", 5, 10);
    RLOG_ERROR("gateway","支付网关调用失败: 支付宝接口返回 code=%s, msg=%s", "SYSTEM_ERROR", "系统繁忙，请稍后再试");
    RLOG_FATAL("gateway","服务雪崩: 依赖服务 %s 不可用，触发降级失败，服务即将退出", "payment-service");
    r_log_flush();
    printf("===================================\n");

    // ============================================
    // 场景6: 现实时间时间戳 —— 安全审计日志
    // ============================================
    r_log_set_time_level(rlv_time_run, 0);
    r_log_set_time_level(rlv_time_local, 1);

    RLOG_TRACE("sec","审计事件: 用户 admin 查看系统日志页面");
    RLOG_DEBUG("sec","登录尝试: 用户名=%s, 来源IP=%s, 方式=%s", "root", "10.0.0.15", "SSH密钥");
    RLOG_INFO("sec","用户登录成功: user_id=%d, username=%s, 登录时间=%s", 1001, "zhangsan", "2024-06-03 09:15:32");
    RLOG_WARN("sec","异常登录: 用户 %s 从非常用地点 %s 登录", "lisi", "北京市");
    RLOG_ERROR("sec","权限拒绝: 用户 %s 尝试访问 %s，所需权限: %s", "wangwu", "/admin/config", "ROLE_SUPER_ADMIN");
    RLOG_FATAL("sec","安全告警: 检测到暴力破解攻击，源IP %s 在 %d 分钟内失败 %d 次", "192.168.1.100", 5, 1000);
    r_log_flush();
    printf("===================================\n");

    // ============================================
    // 场景7: 现实时间微缩 —— 数据库操作
    // ============================================
    r_log_set_time_level(rlv_time_run, 0);
    r_log_set_time_level(rlv_time_local, 2);

    RLOG_TRACE("db","SQL解析: SELECT id, name, email FROM users WHERE status = 'active'");
    RLOG_DEBUG("db","连接池状态: 活跃连接=%d, 空闲连接=%d, 最大连接=%d", 15, 5, 20);
    RLOG_INFO("db","执行SQL: INSERT INTO orders (user_id, total) VALUES (%d, %.2f), 影响行数=%d", 10086, 299.99, 1);
    RLOG_WARN("db","慢查询告警: SQL执行耗时 %.2fms, 阈值=%dms, SQL=%s", 3200.0, 1000, "SELECT * FROM logs WHERE create_time > '2024-01-01'");
    RLOG_ERROR("db","数据库连接断开: host=%s, port=%d, 错误: %s", "db-master.internal", 3306, "Connection reset by peer");
    RLOG_FATAL("db","主库宕机: 无法连接到数据库主节点，切换从库失败，服务停止");
    r_log_flush();
    printf("===================================\n");

    // ============================================
    // 场景8: 现实时间完全 —— 系统监控告警
    // ============================================
    r_log_set_time_level(rlv_time_run, 0);
    r_log_set_time_level(rlv_time_local, 3);

    RLOG_TRACE("mon","采集指标: cpu_usage=45.2%, memory_usage=67.8%, disk_io_read=120MB/s");
    RLOG_DEBUG("mon","进程监控: PID=%d, name=%s, cpu=%.1f%%, mem=%.1f%%, threads=%d", 18432, "myapp-server", 12.5, 8.3, 24);
    RLOG_INFO("mon","健康检查通过: 服务=%s, 状态=%s, 响应时间=%.2fms", "api-gateway", "UP", 15.6);
    RLOG_WARN("mon","CPU使用率告警: 当前 %.1f%%, 阈值 %.1f%%, 持续 %d 分钟", 85.3, 80.0, 3);
    RLOG_ERROR("mon","内存溢出风险: 堆内存使用 %.1f%% / %.1f GB, 老年代GC次数=%d", 92.5, 4.0, 15);
    RLOG_FATAL("mon","系统崩溃: OOM Killer 触发，进程 %s(PID=%d) 被终止", "myapp-server", 18432);
    r_log_flush();
    printf("===================================\n");

    // ============================================
    // 场景9: 运行时间微缩 + 现实时间时间戳 —— 消息队列
    // ============================================
    r_log_set_time_level(rlv_time_run, 1);
    r_log_set_time_level(rlv_time_local, 1);

    RLOG_TRACE("mq","消息入队: topic=%s, partition=%d, offset=%ld", "order-events", 2, 154320);
    RLOG_DEBUG("mq","消费者组状态: group_id=%s, members=%d, lag=%ld", "order-processor", 3, 500);
    RLOG_INFO("mq","消息消费: topic=%s, key=%s, value_size=%d bytes", "order-events", "ORD_001", 256);
    RLOG_WARN("mq","消费延迟: 当前 lag=%ld, 阈值=%d, 消费者 %s 处理缓慢", 5000, 1000, "consumer-02");
    RLOG_ERROR("mq","消息发送失败: broker=%s:%d, topic=%s, 错误: %s", "kafka-01", 9092, "order-events", "NotEnoughReplicas");
    RLOG_FATAL("mq","Kafka集群不可用: 所有broker连接失败，消息队列服务停止");
    r_log_flush();
    printf("===================================\n");

    // ============================================
    // 场景10: 运行时间时间戳 + 现实时间微缩 —— 缓存系统
    // ============================================
    r_log_set_time_level(rlv_time_run, 2);
    r_log_set_time_level(rlv_time_local, 2);

    RLOG_TRACE("cache","缓存查询: key=%s, hit=%s", "user:10086:profile", "true");
    RLOG_DEBUG("cache","Redis连接池: 活跃=%d, 空闲=%d, 等待=%d", 8, 4, 2);
    RLOG_INFO("cache","缓存更新: key=%s, ttl=%ds, size=%d bytes", "product:8848:stock", 3600, 128);
    RLOG_WARN("cache","缓存穿透: key=%s 在 %d 秒内被查询 %d 次，结果均为空", "user:99999", 60, 500);
    RLOG_ERROR("cache","Redis命令超时: GET %s, 等待 %.2fms, 超时阈值 %dms", "session:abc123", 5500.0, 5000);
    RLOG_FATAL("cache","缓存雪崩: 大量key同时过期，数据库压力激增，服务响应时间 > %dms", 30000);
    r_log_flush();
    printf("===================================\n");

    // ============================================
    // 场景11: 运行时间缩略 + 现实时间完全 —— 容器编排
    // ============================================
    r_log_set_time_level(rlv_time_run, 3);
    r_log_set_time_level(rlv_time_local, 3);

    RLOG_TRACE("k8s","Pod事件: namespace=%s, pod=%s, event=%s", "production", "web-app-7d9f4b8c5-x2k9m", "Scheduled");
    RLOG_DEBUG("k8s","容器状态: pod=%s, container=%s, status=%s, restarts=%d", "web-app-7d9f4b8c5-x2k9m", "nginx", "Running", 0);
    RLOG_INFO("k8s","服务扩容: deployment=%s, replicas=%d -> %d, 原因=%s", "web-app", 3, 6, "CPU利用率超过80%");
    RLOG_WARN("k8s","资源紧张: 节点 %s CPU请求率 %.1f%%, 内存请求率 %.1f%%", "k8s-node-03", 95.0, 88.0);
    RLOG_ERROR("k8s","Pod崩溃: pod=%s, exit_code=%d, reason=%s", "api-server-5c4d8e2f1-a1b2c", 137, "OOMKilled");
    RLOG_FATAL("k8s","集群故障: 控制平面 etcd 集群失去仲裁，所有调度操作停止");
    r_log_flush();
    printf("===================================\n");

    // ============================================
    // 场景12: 运行时间完全 + 现实时间完全 —— 全链路压测
    // ============================================
    r_log_set_time_level(rlv_time_run, 4);
    r_log_set_time_level(rlv_time_local, 3);

    RLOG_TRACE("perf","压测请求: thread_id=%d, iteration=%d, api=%s", 42, 1567, "POST /api/v1/order");
    RLOG_DEBUG("perf","请求构造: body_size=%d, headers=%d, content_type=%s", 1024, 8, "application/json");
    RLOG_INFO("perf","压测指标: QPS=%.1f, 平均延迟=%.2fms, P99=%.2fms, 错误率=%.2f%%", 1250.5, 45.3, 120.8, 0.05);
    RLOG_WARN("perf","性能退化: 当前P99=%.2fms, 基线P99=%.2fms, 退化比例=%.1f%%", 120.8, 80.0, 51.0);
    RLOG_ERROR("perf","压测失败: 请求 %s 返回 HTTP %d, 响应体: %s", "POST /api/v1/pay", 502, "Bad Gateway");
    RLOG_FATAL("perf","压测终止: 错误率 %.2f%% 超过阈值 %.2f%%, 自动停止压测任务", 15.0, 5.0);
    r_log_flush();
    printf("===================================\n");
}



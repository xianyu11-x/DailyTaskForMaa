# DailyTaskForMAA

MAA的日常任务调度后端，实现每日任务关卡选择自动化

## Quick Start

需求：安装docker和docker-compose

```
sudo ./start.sh
```

## 支持功能

1.MAA远程调用必须的后端端点

2.用户每日任务策略的配置和查询(目前sideStory策略是默认倒数第2关)

3.自动从MAAResource拉取支持的SideStory并推送关卡信息

TODO: quickTask 快速发起一组自定义任务列表(肉鸽刷钱)

TODO:gui+用户管理

## 第三方库

协程库+网络库：[co_async](https://github.com/archibate/co_async)

json库：[rapidjson](https://github.com/Tencent/rapidjson)

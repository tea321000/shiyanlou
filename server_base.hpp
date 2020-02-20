//
// server_base.hpp
// web_server
// created by changkun at shiyanlou.com
//

#ifndef SERVER_BASE_HPP
#define SERVER_BASE_HPP

#include <boost/asio.hpp>

#include <regex>
#include <unordered_map>
#include <thread>

namespace ShiyanlouWeb {
    struct Request {
        // 请求方法, POST, GET; 请求路径; HTTP 版本
        std::string method, path, http_version;
        // 对 content 使用智能指针进行引用计数
        std::shared_ptr<std::istream> content;
        // 哈希容器, key-value 字典
        std::unordered_map<std::string, std::string> header;
        // 用正则表达式处理路径匹配
        std::smatch path_match;
    };

    // 使用 typedef 简化资源类型的表示方式
    typedef std::map<std::string, std::unordered_map<std::string,
    std::function<void(std::ostream&, Request&)>>> resource_type;

    // socket_type 为 HTTP or HTTPS
    template <typename socket_type>
    class ServerBase {
    public:
        resource_type resource;
        resource_type default_resource;

        // 构造服务器, 初始化端口, 默认使用一个线程
        ServerBase(unsigned short port, size_t num_threads=1) :
            endpoint(boost::asio::ip::tcp::v4(), port),
            acceptor(m_io_service, endpoint),
            num_threads(num_threads) {}

        void start();
    protected:
        // asio 库中的 io_service 是调度器，所有的异步 IO 事件都要通过它来分发处理
        // 换句话说, 需要 IO 的对象的构造函数，都需要传入一个 io_service 对象
        boost::asio::io_service m_io_service;
        // IP 地址、端口号、协议版本构成一个 endpoint，并通过这个 endpoint 在服务端生成
        // tcp::acceptor 对象，并在指定端口上等待连接
        boost::asio::ip::tcp::endpoint endpoint;
        // 所以，一个 acceptor 对象的构造都需要 io_service 和 endpoint 两个参数
        boost::asio::ip::tcp::acceptor acceptor;

        // 服务器线程
        size_t num_threads;
        std::vector<std::thread> threads;

        // 所有的资源及默认资源都会在 vector 尾部添加, 并在 start() 中创建
        std::vector<resource_type::iterator> all_resources;

        // 需要不同类型的服务器实现这个方法
        virtual void accept() {}

        // 处理请求和应答
        void process_request_and_respond(std::shared_ptr<socket_type> socket) const;
    };

    template<typename socket_type>
    class Server : public ServerBase<socket_type> {};
}
#endif /* SERVER_BASE_HPP */

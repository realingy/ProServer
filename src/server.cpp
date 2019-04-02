/**
 * @file epollserver.cpp
 * @brief  
 * @author  
 * @version 1.0
 * @date 2019年02月26日 15时00分18秒
 */

#include "util/log.h"
#include "fde.h"
#include "server.h"

const static int DEFAULT_TYPE = 0;
const static int HANDLER_TYPE = 1;

Server::Server()
    : serv_link_(NULL)
    , link_count_(0)
{
    m_epollfd = epoll_create(EPOLLSIZE); //创建epoll文件描述符
    memset(m_buf, 0, sizeof(m_buf));

    const char *ip = "127.0.0.1";
    int port = 6666;
    bind_and_listen(ip, port); //绑定加监听
    do_epoll(); //
}

Server::~Server()
{
    if(m_epollfd) {
        ::close(m_epollfd);
    }
    if(serv_link_) {
        delete serv_link_;
        serv_link_ = NULL;
    }
}

void Server::loop()
{
    while(1) {
        if(single_loop() == -1)
            break;
    }
}

int Server::single_loop()
{
#if 0
    const Fdevents::events_t *events;
    events = fdes->wait(20);
    if(events == NULL){
        log_fatal("events.wait error: %s", strerror(errno));
        return 0;
    }

    for(int i=0; i<(int)events->size(); i++){
        const Fdevent *fde = events->at(i);
        if(fde->data.ptr == serv_link_){
            this->accept_session();
        }else if(fde->data.num == HANDLER_TYPE){
            Handler *handler = (Handler *)fde->data.ptr;
            while(Response *resp = handler->handle()){
                Session *sess = this->get_session(resp->sess.id);
                if(sess){
                    Link *link = sess->link;
                    link->send(resp->msg);
                    if(link && !link->output.empty()){
                        fdes->set(link->fd(), FDEVENT_OUT, DEFAULT_TYPE, sess);
                    }
                }
                delete resp;
            }
        }else{
            Session *sess = (Session *)fde->data.ptr;
            Link *link = sess->link;
            if(fde->events & FDEVENT_IN){
                if(this->read_session(sess) == -1){
                    continue;
                }
            }
            if(fde->events & FDEVENT_OUT){
                if(this->write_session(sess) == -1){
                    continue;
                }
            }
            if(link && !link->output.empty()){
                fdes->set(link->fd(), FDEVENT_OUT, DEFAULT_TYPE, sess);
            }
        }
    }
#endif
    return 0;
}

int Server::bind_and_listen(const char *ip, int port)
{
    serv_link_ = Link::listen(ip, port);
    if(!serv_link_)
        return -1;
    
    m_listenfd = serv_link_->getSock();
    return 0;
}

void Server::add_event(int fd, int state)
{
    struct epoll_event event;
    event.events = state;
    event.data.fd = fd;
    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event);
}

//接收新的client连接,并将client文件描述符添加到监控描述符中
void Server::handle_accept()
{
    int clientfd = serv_link_->accept();
    add_event(clientfd, EPOLLIN);
}

void Server::delete_event(int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &ev);
}

void Server::modify_event(int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &ev);
}

void Server::do_read(int fd)
{
    int nread;
    nread = read(fd, m_buf, MAXLINE);
    if(-1 == nread) {
        perror("read error!");
        close(fd);
        delete_event(fd, EPOLLIN);
    } else if(!nread) {
        fprintf(stderr, "client error.\n");
        close(fd);
        delete_event(fd, EPOLLIN);
    } else {
        printf("read message is: %s", m_buf);
        modify_event(fd, EPOLLOUT);
    }
}

void Server::do_write(int fd)
{
    int nwrite;
    nwrite = write(fd, &m_buf, strlen(m_buf));
    if(-1 == nwrite) {
        perror("write error!");
        close(fd);
        delete_event(fd, EPOLLOUT);
    } else {
        modify_event(fd, EPOLLIN);
    }
    memset(m_buf, 0, MAXLINE);
}

//处理epoll_wait等到的事件，并处理事件:
// 1.添加新的client连接
// 2.读取client发来的包
// 3.发包
void Server::handle_events(int num)
{
    int fd;
    for(int i = 0; i < num; i++) {
        fd = m_events[i].data.fd;
        if((fd == m_listenfd) && (m_events[i].events & EPOLLIN)) 
            handle_accept(); //添加新的client连接
        else if(m_events[i].events & EPOLLIN)
            do_read(fd); //读取旧client的包
        else if(m_events[i].events & EPOLLOUT)
            do_write(fd); //发包
    }
}

void Server::do_epoll()
{
    add_event(m_listenfd, EPOLLIN); //先将listenfd添加到监听事件组中
    int ret;
    while(1) {
        ret = epoll_wait(m_epollfd, m_events, EVENTS, -1);
        handle_events(ret);
    }
}

#if 0
void Server::add_handler(Handler *handler)
{
    handler->m_init();
    handlers.push_back(handler);
    if(handler->fd() > 0) {
/*
        fdes->set(handler->fd(), FDEVENT_IN, HANDLER_TYPE, handler);
*/
    }
}
#endif

#if 0
int Server::close_session(Session *sess)
{
    Link *link = sess->link;
    for(int i=0; i<this->handlers.size(); i++){
        Handler *handler = this->handlers[i];
        handler->close(*sess);
    }

    this->link_count --;
    log_debug("delete link %s:%d, fd: %d, links: %d",
        link->remote_ip, link->remote_port, link->fd(), this->link_count);
    fdes->del(link->fd());

    this->sessions.erase(sess->id);
    delete link;
    delete sess;
    return 0;
}

int Server::read_session(Session *sess)
{
    Link *link = sess->link;
    if(link->error()){
        return 0;
    }

    int len = link->read();
    if(len <= 0){
        this->close_session(sess);
        return -1;
    }

    while(1) {
        Request req;
        int ret = link->recv(&req.msg);
        if(ret == -1){
            log_info("fd: %d, parse error, delete link", link->fd());
            this->close_session(sess);
            return -1;
        }else if(ret == 0){
            // 报文未就绪, 继续读网络
            break;
        }
        req.stime = microtime();
        req.sess = *sess;
	
        Response resp;
        for(int i=0; i<this->handlers.size(); i++){
            Handler *handler = this->handlers[i];
            req.time_wait = 1000 * (microtime() - req.stime);
            HandlerState state = handler->proc(req, &resp);
            req.time_proc = 1000 * (microtime() - req.stime) - req.time_wait;
            if(state == HANDLE_RESP){
                link->send(resp.msg);
                if(link && !link->output.empty()){
                    fdes->set(link->fd(), FDEVENT_OUT, DEFAULT_TYPE, sess);
                }
		
                if(log_level() >= Logger::LEVEL_DEBUG){
                    log_debug("w:%.3f,p:%.3f, req: %s resp: %s",
                        req.time_wait, req.time_proc,
                        msg_str(req.msg).c_str(),
                        msg_str(resp.msg).c_str());
                }
            }else if(state == HANDLE_FAIL){
                this->close_session(sess);
                return -1;
            }
        }
    }

    return 0;
}

int Server::write_session(Session *sess)
{
	Link *link = sess->link;
	if(link->error()){
		return 0;
	}

	int len = link->write();
	if(len <= 0){
		log_debug("fd: %d, write: %d, delete link", link->fd(), len);
		this->close_session(sess);
		return -1;
	}
	if(link->output.empty()){
		fdes->clr(link->fd(), FDEVENT_OUT);
	}
	return 0;
}
#endif


#if 0
//删除事件
int del(int fd)
{
    struct epoll_event epe;
    int ret = epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &epe);

    if(-1 == ret) {

    }

//////////////////////////////////////////////////////////
    if(!(fde->s_flags) & flags) {
        return 0;
    }

    fde->s_flags &= ~flags;

    int ctl_op = fde->s_flags ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
//////////////////////////////////////////////////////////
}

//清空事件
int clr(int fd, int flags)
{
    struct Fdevent *fde = get_fde(fd);
    if(!(fde->s_flags) & flags) {
        return 0;
    }

    fde->s_flags &= ~flags;

    int ctl_op = fde->s_flags ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    struct epoll_event epe;
    epe.data.ptr = fde;
    epe.events = 0;
    if(fde->s_flags & FDEVENT_IN) 
        epe.events |= EPOLLIN;
    if(fde->s_flags & FDEVENT_OUT) 
        epe.events |= EPOLLOUT;

    int ret = epoll_ctl(m_epollfd, ctl_op, fd, &epe);
    if(ret == -1) {
        return -1;
    }
    return 0;
}

//等待客户端事件,将已就绪事件清空,加载新的就绪事件
const int wait(int timeout) {
    struct Fdevent *fde;
    struct epoll_event *epe;
    ready_events.clear();

    int ret = epoll_wait(m_epollfd, m_events, EVENTS, timeout);
    if(-1 == ret) {
        return NULL;
    }

    for(int i = 0; i < ret; i++) {
        epe = &m_events[i];
        fde = (struct Fdevent *)epe->data.ptr;

        fde->events = FDEVENT_NONE;
        if(epe->events & EPOLLIN)
            fde->events |= FDEVENT_IN;
        else if(epe->events & EPOLLPRI)
            fde->events |= FDEVENT_IN;
        else if(epe->events & EPOLLOUT)
            fde->events |= FDEVENT_OUT;
        else if(epe->events & EPOLLHUP)
            fde->events |= FDEVENT_ERR;
        else if(epe->events & EPOLLERR)
            fde->events |= FDEVENT_ERR;
        ready_events.push_back(fde);
    }
    return &ready_events;
}
#endif



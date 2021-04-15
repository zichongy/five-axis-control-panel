#include "video_adapter.h"
#include "ui_video_adapter.h"

#include "moc_video_adapter.cpp"

OpenCVAdapter::OpenCVAdapter(QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
}

OpenCVAdapter::~OpenCVAdapter()
{
    is_camera_stop_ = true;
}

void OpenCVAdapter::run(){
    cv::VideoCapture cv_capture_;
    cv::Mat frame;
    bool camera_open_success_ = cv_capture_.open(camera_index_); //从指定摄像头读入
    qDebug() << "Camera open success: " << camera_open_success_;

    if (camera_open_success_)
    {
        //设置并获取相机分辨率和帧率
        cv_capture_.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        cv_capture_.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
        int width = cv_capture_.get(cv::CAP_PROP_FRAME_WIDTH);
        int height = cv_capture_.get(cv::CAP_PROP_FRAME_HEIGHT);
        int rate = cv_capture_.get(cv::CAP_PROP_FPS);
        qDebug() << "Camera has the resolution of " << width << " x " << height
                 << " with a frame rate of " << rate << " FPS.";
        //循环送帧
        while (!is_camera_stop_)
        {
            cv_capture_ >> frame;
            if(frame.empty())
            {
                qDebug() << "OpenCVAdapter: 解析失败: 帧为空。";
                continue;
            }
            else if(!frame.empty())
            {
                emit sigSendFrame(frame);
            }
        }
    }
}

void OpenCVAdapter::cvFrameInitialize(bool is_camera_perm_,int desired_camera_index_){
    is_camera_stop_ = true;
    wait();
    is_camera_stop_ = !is_camera_perm_;
    qDebug() << "Camera is stop: " << is_camera_stop_;
    camera_index_ = desired_camera_index_;
    if (!is_camera_stop_) start();
}

VideoAdapter::VideoAdapter(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoAdapter)
{
    //User Interface
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(1);

    //Camera
    m_thread = new OpenCVAdapter(this);
    connect(m_thread, SIGNAL(sigSendFrame(cv::Mat)),this,SLOT(slotGetFrame(cv::Mat)));
    connect(this, SIGNAL(sigCVInitialize(bool,int)),m_thread,SLOT(cvFrameInitialize(bool,int)));

    //Frame Analyze
    //Load the dictionary that was used to generate the markers.
    dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    //Initialize the detector parameters using default values
    parameters = cv::aruco::DetectorParameters::create();
}

VideoAdapter::~VideoAdapter(){
    emit sigCVInitialize(false,0);//发送信号关闭相机，结束线程
    m_thread->wait();
    delete m_thread;
    delete ui;
}

void VideoAdapter::cameraStatusChanged(bool camera_permission, int desired_camera_index){
    is_camera_open_ = camera_permission;
    emit sigCVInitialize(camera_permission,desired_camera_index);
    frame_countdown_ = frame_interval_;

    if(is_camera_open_)
    {
        ui->stackedWidget->setCurrentIndex(0);
    }
    else if(!is_camera_open_)
    {
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void VideoAdapter::slotGetFrame(cv::Mat mat_src_){
    //隔frame_interval_帧处理一次
    if (frame_countdown_ == frame_interval_){
        frame_countdown_ -= 1;
        mat_src_ = framePosAnalyze(mat_src_,true);
    }
    else if (frame_countdown_ > 1){
        frame_countdown_ -= 1;
        mat_src_ = framePosAnalyze(mat_src_,false);
    }
    else{
        frame_countdown_ = frame_interval_;
    }

    //送去显示
    QImage src_image_ = cvMat2QImage(mat_src_);
    QPixmap original_pixmap_ = QPixmap::fromImage(src_image_);
    QPixmap scaled_pixmap_ = original_pixmap_.scaled(ui->label->width(),ui->label->height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->label->setPixmap(scaled_pixmap_);
}

QImage VideoAdapter::cvMat2QImage(const cv::Mat mat){
    if (mat.type() == CV_8UC1)
    {
        QImage image(mat.cols,mat.rows,QImage::Format_Indexed8);
        image.setColorCount(256);
        for (int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i,i,i));
        }

        uchar *pSrc = mat.data;
        for (int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    else if (mat.type() == CV_8UC3)
    {
        const uchar *pSrc = (const uchar*)mat.data;
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
        //8位，通道数为4
    else if (mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";

        const uchar *pSrc = (const uchar*)mat.data;
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
        //32位，通道数为3
    else if (mat.type() == CV_32FC3)
    {
        cv::Mat temp;
        mat.convertTo(temp, CV_8UC3);

        const uchar *pSrc = (const uchar*)temp.data;
        QImage image(pSrc, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

cv::Mat VideoAdapter::framePosAnalyze(cv::Mat frame_,bool is_key_frame_)
{
    //cv::Mat frame_copy_ = frame_.clone(); //深拷贝
    cv::Mat frame_copy_ = frame_; //浅拷贝

    //是关键帧则刷新检测
    if(is_key_frame_){
        cv::aruco::detectMarkers(frame_, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
        //输出检测结果
        if(markerIds.size() != 0) qDebug() << markerIds;
        else qDebug() << "No markers find in this frame.";
    }

    //如果有检测到的ArUco标记，将其画出，否则直接返回原图
    if(markerIds.size() != 0){
        qDebug() << markerIds;
        cv::aruco::drawDetectedMarkers(frame_copy_,markerCorners,markerIds);
        return frame_copy_;
    }
    else return frame_;
    return frame_;
}

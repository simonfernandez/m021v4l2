#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

#include <stdio.h>
#include <sys/timeb.h>

#include "m021_v4l2.h"

static pthread_t video_thread;
static pthread_mutex_t lock;
static int count;

// http://www.firstobject.com/getmillicount-milliseconds-portable-c++.htm
static int getMilliCount(){

    timeb tb;
    ftime(&tb);
    int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
    return nCount;
}

static void * loop(void * arg)
{
    m021_800x460_t cap;
    m021_800x460_init(0, &cap);

    Mat * mat = (Mat *)arg;

    while (true) {

        pthread_mutex_lock(&lock);

        m021_800x460_grab_bgr(&cap, mat->data);

        pthread_mutex_unlock(&lock);

        count++;
    }

    return (void *)0;
}

static void run(Mat & mat)
{
    mat = Mat(460, 800, CV_8UC3);

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init failed\n");
        exit(1);
    }

    if (pthread_create(&video_thread, NULL, loop, &mat)) {
        fprintf(stderr, "Failed to create thread\n");
        exit(1);
    }
}

int main()
{
    Mat mat;

    run(mat);

    cvNamedWindow("LI-USB30-M021", CV_WINDOW_AUTOSIZE);

    int start = getMilliCount();

    while (true) {

        imshow("LI-USB30-M021", mat);

        if (cvWaitKey(1) == 27) 
            break;

    }

    double duration = (getMilliCount() - start) / 1000.;

    printf("%d frames in %3.3f seconds = %3.3f frames /sec \n", count, duration, count/duration);

    return 0;
}

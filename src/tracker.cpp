#include "tracker.hpp"
#include <opencv2/opencv.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <filesystem>

/* ------------ helpers --------------------------------------------------- */
static inline float norm(float v, int dim) { return (v > 1.f) ? v / dim : v; }

static float euclid(const Detection& d, const TrackedObject& t,
                    int W = 640, int H = 480)
{
    float dx = norm(d.x, W) - norm(t.x, W);
    float dy = norm(d.y, H) - norm(t.y, H);
    return std::sqrt(dx * dx + dy * dy);          // 0–1 space
}

/* ------------ ctor ------------------------------------------------------- */
Tracker::Tracker(int max_missing, float max_dist)
    : next_id_(0), max_missing_(max_missing), max_dist_(max_dist) {}

/* ------------ update ----------------------------------------------------- */
std::vector<TrackedObject> Tracker::update(const std::vector<Detection>& dets)
{
    std::vector<bool> matched(tracked_.size(), false);

    for (const auto& d : dets)
    {
        float best = max_dist_;
        int   idx  = -1;

        for (size_t i = 0; i < tracked_.size(); ++i)
        {
            if (matched[i]) continue;
            float dist = euclid(d, tracked_[i]);
            if (dist < best) { best = dist; idx = static_cast<int>(i); }
        }

        if (idx != -1) {                         // update existing
            auto& t = tracked_[idx];
            t.x = d.x;  t.y = d.y;
            t.width = d.width;  t.height = d.height;
            t.last_seen = 0;
            t.history.emplace_back(int(d.x), int(d.y));
            matched[idx] = true;
        } else {                                 // new track
            TrackedObject t{ next_id_++, d.x, d.y,
                              d.width, d.height, 0,
                              { cv::Point(int(d.x), int(d.y)) } };
            tracked_.push_back(t);
            matched.push_back(true);             // keep sizes equal
        }
    }

    for (size_t i = 0; i < tracked_.size(); ++i)
        if (!matched[i]) tracked_[i].last_seen++;

    tracked_.erase(std::remove_if(tracked_.begin(), tracked_.end(),
        [this](const TrackedObject& t){ return t.last_seen > max_missing_; }),
        tracked_.end());

    return tracked_;
}

/* ------------ visualize -------------------------------------------------- */
cv::Mat Tracker::visualize(const std::vector<TrackedObject>& objs,
                           int frame_no, int W, int H) const
{
    cv::Mat img(H, W, CV_8UC3, cv::Scalar(19, 69, 139)); // Brown background (like soil)

    for (const auto& o : objs)
    {
        bool px = (o.x > 1.f || o.y > 1.f || o.width > 1.f || o.height > 1.f);
        float cx = px ? o.x : o.x * W;
        float cy = px ? o.y : o.y * H;
        float ww = px ? o.width  : o.width  * W;
        float hh = px ? o.height : o.height * H;

        // Draw the tracking rectangle
        cv::Rect r(int(cx - ww / 2), int(cy - hh / 2), int(ww), int(hh));
        cv::rectangle(img, r, cv::Scalar(0, 255, 0), 2); // Green rectangle
        cv::putText(img, "ID:" + std::to_string(o.id),
                    {r.x, r.y - 5}, cv::FONT_HERSHEY_SIMPLEX,
                    0.5, cv::Scalar(0, 0, 255), 1); // Red text for ID

        // Draw motion lines
        for (size_t i = 1; i < o.history.size(); ++i)
            cv::line(img, o.history[i-1], o.history[i], cv::Scalar(255, 0, 0), 1); // Blue motion lines
    }

    // Add frame number label
    std::string label = (frame_no >= 0)
                        ? "Frame " + std::to_string(frame_no)
                        : "Summary";
    cv::putText(img, label, {10, 30}, cv::FONT_HERSHEY_SIMPLEX,
                1.0, cv::Scalar(0, 0, 0), 2); // Black label for frame number

    return img;
}

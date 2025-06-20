#include <iostream>
#include <fstream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <json.hpp>
#include "tracker.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

int main(int argc, char* argv[])
{
    std::string in_path, out_path, vis_dir;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--input")   in_path  = argv[++i];
        else if (arg == "--output") out_path = argv[++i];
        else if (arg == "--vis-dir") vis_dir = argv[++i];
    }
    if (in_path.empty() || out_path.empty() || vis_dir.empty()) {
        std::cerr << "Missing --input / --output / --vis-dir\n";
        return 1;
    }

    fs::create_directories(vis_dir);

    std::ifstream ifs(in_path);
    json frames;  ifs >> frames;

    Tracker tracker;                 // default: max_missing=3, max_dist=0.1
    std::vector<TrackedObject> last_tracked;
    json out_json = json::array();

    for (const auto& f : frames)
    {
        int frame_id = f["frame_id"];
        std::string ts = f["timestamp"];

        std::vector<Detection> dets;
        for (const auto& d : f["detections"])
            dets.push_back({ d["x"], d["y"], d["width"], d["height"] });

        auto tracked = tracker.update(dets);
        last_tracked = tracked;                       // keep for summary

        cv::Mat img = tracker.visualize(tracked, frame_id);
        cv::imwrite(vis_dir + "/frame_" + std::to_string(frame_id) + ".png", img);

        json fout;
        fout["frame_id"] = frame_id;
        fout["timestamp"] = ts;
        for (const auto& t : tracked)
            fout["tracked_objects"].push_back({
                {"id", t.id}, {"x", t.x}, {"y", t.y},
                {"width", t.width}, {"height", t.height}
            });
        out_json.push_back(fout);
    }

    /* -------------- summary image ---------------------------------------- */
    cv::Mat summary = tracker.visualize(last_tracked, -1);
    cv::imwrite(vis_dir + "/summary.png", summary);

    std::ofstream ofs(out_path);
    ofs << out_json.dump(2);
    std::cout << "Wrote " << frames.size()
              << " frames + summary to " << vis_dir << '\n';
    return 0;
}
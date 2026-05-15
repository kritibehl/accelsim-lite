#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct Shape {
    std::string id;
    std::vector<std::string> layout;
};

struct Access {
    std::string site;
    std::string shape_id;
    std::string property;
};

struct SiteStats {
    std::string site;
    int hits = 0;
    int misses = 0;
    int generic_fallbacks = 0;
    int shape_invalidations = 0;
    std::string state = "uninitialized";
    std::set<std::string> seen_shapes;
};

std::vector<Shape> shapes = {
    {"shape_xy", {"x", "y"}},
    {"shape_xyz", {"x", "y", "z"}},
    {"shape_yx", {"y", "x"}},
    {"shape_x", {"x"}},
    {"shape_wx", {"w", "x"}}
};

std::vector<Access> trace = {
    {"load_x", "shape_xy", "x"},
    {"load_x", "shape_xy", "x"},
    {"load_x", "shape_xyz", "x"},
    {"load_x", "shape_yx", "x"},
    {"load_x", "shape_x", "x"},
    {"load_x", "shape_wx", "x"},
    {"load_y", "shape_xy", "y"},
    {"load_y", "shape_xy", "y"},
    {"load_y", "shape_xyz", "y"},
    {"load_y", "shape_yx", "y"}
};

std::string classify_state(size_t shape_count) {
    if (shape_count == 0) return "uninitialized";
    if (shape_count == 1) return "monomorphic";
    if (shape_count <= 3) return "polymorphic";
    return "megamorphic_generic_fallback";
}

int main() {
    std::map<std::string, SiteStats> stats;

    for (const auto& access : trace) {
        auto& s = stats[access.site];
        s.site = access.site;

        bool seen_before = s.seen_shapes.count(access.shape_id) > 0;
        std::string old_state = s.state;

        if (seen_before && s.state != "megamorphic_generic_fallback") {
            s.hits++;
        } else {
            s.misses++;
            s.seen_shapes.insert(access.shape_id);
        }

        s.state = classify_state(s.seen_shapes.size());

        if (old_state != "uninitialized" && old_state != s.state) {
            s.shape_invalidations++;
        }

        if (s.state == "megamorphic_generic_fallback") {
            s.generic_fallbacks++;
        }
    }

    std::ofstream json("execution_engine_lab/property_access_profiles.json");
    json << "{\n";
    json << "  \"scope\": \"toy JavaScript-like object shape and inline-cache simulation; not V8 implementation\",\n";
    json << "  \"sites\": [\n";

    int i = 0;
    for (const auto& [site, s] : stats) {
        int total = s.hits + s.misses;
        double fast_pct = total ? (100.0 * s.hits / total) : 0.0;

        json << "    {\n";
        json << "      \"site\": \"" << site << "\",\n";
        json << "      \"final_state\": \"" << s.state << "\",\n";
        json << "      \"cache_hits\": " << s.hits << ",\n";
        json << "      \"cache_misses\": " << s.misses << ",\n";
        json << "      \"fast_path_percent\": " << fast_pct << ",\n";
        json << "      \"generic_fallbacks\": " << s.generic_fallbacks << ",\n";
        json << "      \"shape_invalidations\": " << s.shape_invalidations << ",\n";
        json << "      \"unique_shapes_seen\": " << s.seen_shapes.size() << "\n";
        json << "    }";
        if (++i != (int)stats.size()) json << ",";
        json << "\n";
    }

    json << "  ]\n";
    json << "}\n";

    std::ofstream md("execution_engine_lab/inline_cache_transition_report.md");
    md << "# Inline Cache Transition Report\n\n";
    md << "> Scope: toy JavaScript-like object-shape and inline-cache simulation. This is not V8 internals work.\n\n";
    md << "| Site | Final State | Hits | Misses | Generic Fallbacks | Shape Invalidations | Unique Shapes |\n";
    md << "|---|---|---:|---:|---:|---:|---:|\n";

    for (const auto& [site, s] : stats) {
        md << "| " << site
           << " | " << s.state
           << " | " << s.hits
           << " | " << s.misses
           << " | " << s.generic_fallbacks
           << " | " << s.shape_invalidations
           << " | " << s.seen_shapes.size()
           << " |\n";
    }

    md << "\n## Interpretation\n\n";
    md << "- A single repeated shape behaves like a monomorphic cache.\n";
    md << "- A few shapes move the access site into polymorphic behavior.\n";
    md << "- Too many shapes trigger a megamorphic-style generic fallback.\n";
    md << "- Shape transitions are tracked as invalidation events.\n";
    md << "- This models runtime optimization concepts without claiming V8 implementation experience.\n";

    std::cout << "Generated execution_engine_lab/property_access_profiles.json\n";
    std::cout << "Generated execution_engine_lab/inline_cache_transition_report.md\n";
}

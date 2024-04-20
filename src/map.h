#pragma once
#include "raylib.h"
#include <fstream>
#include <vector>
#include "shapes.h"
#include <iostream>
struct Map {
    std::vector<Shape*> objs;
    char mapfilename[200] = "test.map";
    Map () {

    }
    void clear() {
        for (auto& o : objs) delete(o);
        objs.clear();
    }

    void save() {
        FILE* fid=fopen(mapfilename, "w");
        for (auto& o : objs) {
            fprintf(fid, "%s\n", o->serialize());
        }        
        fclose(fid);
    }

    void load() {
        std::ifstream ifs(mapfilename);

        std::string line;
        while(std::getline(ifs, line)) {

            char shape[25], data[70];
            sscanf(line.c_str(), "%s %[^\n]", shape, data );

            if (strcmp(shape, "RECTANGLE") == 0) {
                Rectangle2 r;
                Color c;
                sscanf(data, "%f %f %f %f COLOR %hhd %hhd %hhd %hhd",
                            &r.x, &r.y, &r.width, &r.height,
                            &c.r, &c.g, &c.b, &c.a);
                auto* tmp = new RectShape();
                tmp->color = c;
                tmp->bounds = r;

                objs.push_back(tmp);
                
            } else if (strcmp(shape,"CIRCLE")==0) {
                Vector2 center;
                float radius;
                Color c;
                sscanf(data, "%f %f %f COLOR %hhd %hhd %hhd %hhd",
                            &center.x, &center.y, &radius,
                            &c.r, &c.g, &c.b, &c.a);
                auto* tmp = new CircleShape();
                tmp->color = c;
                tmp->radius = radius;
                tmp->center = center;
                tmp->setBounds();
                objs.push_back(tmp);
                
            }
        }
    }
};

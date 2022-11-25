#pragma once

#ifndef TAG_H
#define TAG_H

#include <string>


struct Tag {
	std::string tag;
    operator const std::string&() { return tag; }
};

#endif
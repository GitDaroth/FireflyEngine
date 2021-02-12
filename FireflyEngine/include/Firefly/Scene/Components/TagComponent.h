#pragma once

#include <string>

namespace Firefly
{
    struct TagComponent
    {
        std::string m_tag = "Unnamed";

        TagComponent() = default;
        TagComponent(const TagComponent& other) = default;
        TagComponent(const std::string& tag) :
            m_tag(tag) {}
    };
}
#pragma once

namespace Firefly
{
	struct Version
	{
		size_t major;
		size_t minor;
		size_t patch;

		Version(size_t major = 0, size_t minor = 0, size_t patch = 0) 
		{
			this->major = major;
			this->minor = minor;
			this->patch = patch;
		}
	};
}
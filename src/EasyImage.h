/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// INCLUDES & DEFINES////////

#pragma once

#include <VulkanUtils.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// GLOBAL VARS ////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// FUNCS ////////

class EasyImage
{
private:
	int m_width;
	int m_height;
	int m_channels;
	stbi_uc *m_ppixels;
	bool m_loaded = false; 

public:
	EasyImage() {

	}

	EasyImage(const char* path)
	{
		load(path); 
	}

	~EasyImage() {
		destroy(); 
	}

	void load(const char* path)
	{
		if (m_loaded)
		{
			throw std::logic_error("EasyImage was already loaded!"); 
		}

		m_ppixels = stbi_load(path, &m_width, &m_height, &m_channels, STBI_rgb_alpha);

		if (m_ppixels = nullptr) {
			throw std::invalid_argument("Failed to load image or image is corrupted!");
		}

		m_loaded = true; 
	}

	void destroy() {
		if (m_loaded)
		{
			stbi_image_free(m_ppixels);
			m_loaded = false;
		}
	}

	int getWidth() {
		if (!m_loaded) {
			throw std::logic_error("EasyImage was not loaded!");
		}
		return m_width; 
	}

	int getHeight() {
		if (!m_loaded) {
			throw std::logic_error("EasyImage was not loaded!");
		}
		return m_height;
	}

	int getChannels() {
		if (!m_loaded) {
			throw std::logic_error("EasyImage was not loaded!");
		}
		return 4;
	}

	int getSizeInBytes() {
		if (!m_loaded) {
			throw std::logic_error("EasyImage was not loaded!");
		}
		return getWidth() * getHeight() * getChannels();
	}

	stbi_uc *getRaw() {
		if (!m_loaded) {
			throw std::logic_error("EasyImage was not loaded!");
		}
		return m_ppixels;
	}
};
#ifndef AUDIO_CONFIG_H
#define AUDIO_CONFIG_H

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>


/**
 * @brief Handles loading and parsing of .ini audio configuration files
 * Supports reading key-value pairs and arrays organized in sections
 */
class AudioConfig
{
	~AudioConfig()
	{
		ReleaseLoadedFile();
	}

	friend class AudioEngine;

	static constexpr char CATEGORY_SEPARATOR = '.';
	static constexpr char ARRAY_ITEM_SEPARATOR = ',';
	static constexpr char COMMENT_CHAR = '#';
	static constexpr char ALT_COMMENT_CHAR = ';';
	static constexpr char SECTION_START = '[';
	static constexpr char SECTION_END = ']';
	static constexpr char ARRAY_START = '(';
	static constexpr char ARRAY_END = ')';
	static constexpr char KEY_VALUE_SEPARATOR = '=';


	std::ifstream mconfigFile = std::ifstream();

	bool LoadConfigFile(const std::string& filePath)
	{
		mconfigFile.open(filePath);
		if (!mconfigFile.is_open()) { return false; }

		std::string line;
		std::string section;

		while (std::getline(mconfigFile, line))
		{
			// Clean whitespace before and after string
			CleanWhitespace(line);

			// Ignore empty lines or comments
			if (line.empty()
				|| line.starts_with(COMMENT_CHAR)
				|| line.starts_with(ALT_COMMENT_CHAR))
			{
				continue;
			}

			// Parse Sections
			if (line.starts_with(SECTION_START))
			{
				const auto endIndex = line.find(SECTION_END);
				section = line.substr(1, endIndex - 1);
			}

			else
			{
				auto delimIndex = line.find(KEY_VALUE_SEPARATOR);
				if (delimIndex != std::string::npos)
				{
					std::string key = line.substr(0, delimIndex);
					std::string value = line.substr(delimIndex + 1);

					if (value.starts_with(ARRAY_START)) // This is an array
					{
						const auto endIndex = value.find(ARRAY_END);
						const auto rawItems = value.substr(1, endIndex - 1);

						std::vector<std::string> cleanItems;
						std::stringstream stringStream(rawItems);
						std::string currentItem;

						while (std::getline(stringStream, currentItem, ARRAY_ITEM_SEPARATOR))
						{
							cleanItems.push_back(currentItem);
						}

						std::string newEntry = std::format("{}{}{}", section, CATEGORY_SEPARATOR, key);
						mConfigArrayData.emplace(std::pair(newEntry, cleanItems));
					}
					else
					{
						CleanWhitespace(key);
						CleanWhitespace(value);

						std::string newEntry = std::format("{}{}{}", section, CATEGORY_SEPARATOR, key);
						mConfigData.emplace(std::pair(newEntry, value));
					}
				}
			}
		}

		return true;
	}

	void ReleaseLoadedFile()
	{
		if (mconfigFile.is_open())
		{
			mconfigFile.close();
		}
	}

	[[nodiscard]] std::string GetString(const std::string& section, const std::string& key, const std::string& defaultValue = "") const
	{
		const auto it = mConfigData.find(section + CATEGORY_SEPARATOR + key);
		return it == mConfigData.end() ? defaultValue : it->second;
	}

	[[nodiscard]] std::vector<std::string> GetStringArray(const std::string& section, const std::string& key) const
	{
		const auto it = mConfigArrayData.find(section + CATEGORY_SEPARATOR + key);
		return it == mConfigArrayData.end() ? std::vector<std::string>() : it->second;
	}

	[[nodiscard]] int GetInt(const std::string& section, const std::string& key, const int defaultValue = 0) const
	{
		const auto value = GetString(section, key);
		return value.empty() ? defaultValue : std::stoi(value);
	}

	[[nodiscard]] float GetFloat(const std::string& section, const std::string& key, const float defaultValue = 0.0f) const
	{
		const auto value = GetString(section, key);
		return value.empty() ? defaultValue : std::stof(value);
	}

	[[nodiscard]] bool GetBool(const std::string& section, const std::string& key, const bool defaultValue = false) const
	{
		auto value = GetString(section, key);
		if (value.empty()) { return defaultValue; }

		std::ranges::transform(value, value.begin(), tolower);
		return value == "true" || value == "1";
	}

	std::unordered_map<std::string, std::string> mConfigData;
	std::unordered_map<std::string, std::vector<std::string>> mConfigArrayData;

	static void CleanWhitespace(std::string& inString)
	{
		inString.erase(0, inString.find_first_not_of(" \t"));
		inString.erase(inString.find_last_not_of(" \t") + 1);
	}
};
#endif

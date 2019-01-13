#include "config.hpp"

#include "file.hpp"
#include "pkgi.hpp"

static char* skipnonws(char* text, char* end)
{
    while (text < end && *text != ' ' && *text != '\n' && *text != '\r')
    {
        text++;
    }
    return text;
}

static char* skipws(char* text, char* end)
{
    while (text < end && (*text == ' ' || *text == '\n' || *text == '\r'))
    {
        text++;
    }
    return text;
}

static DbSort parse_sort(const char* value, DbSort sort)
{
    if (pkgi_stricmp(value, "title") == 0)
    {
        return SortByTitle;
    }
    else if (pkgi_stricmp(value, "region") == 0)
    {
        return SortByRegion;
    }
    else if (pkgi_stricmp(value, "name") == 0)
    {
        return SortByName;
    }
    else if (pkgi_stricmp(value, "size") == 0)
    {
        return SortBySize;
    }
    else if (pkgi_stricmp(value, "date") == 0)
    {
        return SortByDate;
    }
    else
    {
        return sort;
    }
}

static DbSortOrder parse_order(const char* value, DbSortOrder order)
{
    if (pkgi_stricmp(value, "asc") == 0)
    {
        return SortAscending;
    }
    else if (pkgi_stricmp(value, "desc") == 0)
    {
        return SortDescending;
    }
    else
    {
        return order;
    }
}

static DbFilter parse_filter(char* value, uint32_t filter)
{
    uint32_t result = 0;

    char* start = value;
    for (;;)
    {
        char ch = *value;
        if (ch == 0 || ch == ',')
        {
            *value = 0;
            if (pkgi_stricmp(start, "ASA") == 0)
            {
                result |= DbFilterRegionASA;
            }
            else if (pkgi_stricmp(start, "EUR") == 0)
            {
                result |= DbFilterRegionEUR;
            }
            else if (pkgi_stricmp(start, "JPN") == 0)
            {
                result |= DbFilterRegionJPN;
            }
            else if (pkgi_stricmp(start, "USA") == 0)
            {
                result |= DbFilterRegionUSA;
            }
            else
            {
                return static_cast<DbFilter>(filter);
            }
            if (ch == 0)
            {
                break;
            }
            value++;
            start = value;
        }
        else
        {
            value++;
        }
    }

    return static_cast<DbFilter>(result);
}

Config pkgi_load_config()
{
    try
    {
        Config config;

        config.sort = SortByName;
        config.order = SortAscending;
        config.filter = DbFilterAll;
        config.no_version_check = 0;
        config.install_psp_as_pbp = 0;
        config.custom_config = 0;
        config.install_psp_psx_location = "ux0:";

        char path[256];
        pkgi_snprintf(
                path, sizeof(path), "%s/config.txt", pkgi_get_config_folder());
        LOG("config location: %s", path);

        auto data = pkgi_load(path);//获取配置文件数据（vector<char>类）
        data.push_back('\n');//(在文件尾部增加一个\n防止分割出错)

        LOG("config.txt loaded, parsing");
        auto text = reinterpret_cast<char*>(data.data());//强制类型转换获取字符串数组首地址
        const auto end = text + data.size();//获取配置文件字符串尾指针判断是否读取到EOF

        while (text < end)
        {
            const auto key = text;//保留字符串数组头指针（该字符串指向dict的key）

            text = skipnonws(text, end);//越过字符（获取一个key的长度）
            if (text == end)
                break;

            *text++ = 0;//字符串指针后移一位并标记\0（用以后续strcmp时只对比一个关键词）

            text = skipws(text, end);//越过特殊字符(\n,\r,空格)并将指针指向新地址
            if (text == end)
                break;

            const auto value = text;//保留字符串数组头指针（该字符串指向dict的value）

            text = skipnonws(text, end);//越过字符（获取一个value的长度）
            if (text == end)
                break;

            *text++ = 0;//字符串指针后移一位并标记\0（用以后续获取value值时的截断）

            text = skipws(text, end);//越过特殊字符，为下一次while循环做准备

            if (pkgi_stricmp(key, "url") == 0 ||
                pkgi_stricmp(key, "url_games") == 0)
                config.games_url = value;
            else if (pkgi_stricmp(key, "url_dlcs") == 0)
                config.dlcs_url = value;
            else if (pkgi_stricmp(key, "url_psm_games") == 0)
                config.psm_games_url = value;
            else if (pkgi_stricmp(key, "url_psx_games") == 0)
                config.psx_games_url = value;
            else if (pkgi_stricmp(key, "url_psp_games") == 0)
                config.psp_games_url = value;
            else if (pkgi_stricmp(key, "url_comppack") == 0)
                config.comppack_url = value;
            else if (pkgi_stricmp(key, "url_comppack_index") == 0)
                config.comppack_url = value;
            else if (pkgi_stricmp(key, "sort") == 0)
                config.sort = parse_sort(value, SortByName);
            else if (pkgi_stricmp(key, "order") == 0)
                config.order = parse_order(value, SortAscending);
            else if (pkgi_stricmp(key, "filter") == 0)
                config.filter = parse_filter(value, DbFilterAll);
            else if (pkgi_stricmp(key, "no_version_check") == 0)
                config.no_version_check = 1;
            else if (pkgi_stricmp(key, "install_psp_as_pbp") == 0)
                config.install_psp_as_pbp = 1;
            else if (pkgi_stricmp(key, "install_psp_psx_location") == 0)
                config.install_psp_psx_location = value;
            else if (pkgi_stricmp(key, "custom_config") == 0)
                config.custom_config = 1;
            else if (
                    pkgi_stricmp(key, "psm_disclaimer_yes_i_read_the_readme") ==
                    0)
                config.psm_readme_disclaimer =
                        (pkgi_stricmp(value, "NoPsmDrm") == 0);
        }
        if (config.custom_config==0){
            config.games_url="http://47.100.37.250/tsv_files/PSV_GAMES_SC.tsv";
            config.dlcs_url="http://47.100.37.250/tsv_files/PSV_DLCS.tsv";
            config.psm_games_url="http://47.100.37.250/tsv_files/PSM_GAMES.tsv";
            config.psx_games_url="http://47.100.37.250/tsv_files/PSX_GAMES.tsv";
            config.psp_games_url="http://47.100.37.250/tsv_files/PSP_GAMES.tsv";
            config.comppack_index_url="http://47.100.37.250/tsv_files/"
            config.comppack_url="https://gitlab.com/nopaystation_repos/nps_compati_packs/raw/master/";
        }
        if ((!config.comppack_url.empty())&&config.comppack_index_url.empty())
            config.comppack_index_url=config.comppack_url;
        return config;
    }
    catch (const std::exception& e)
    {
        throw formatEx<std::runtime_error>(
                "加載配置失敗:\n{}", e.what());
    }
}

static const char* sort_str(DbSort sort)
{
    switch (sort)
    {
    case SortByTitle:
        return "title";
    case SortByRegion:
        return "region";
    case SortByName:
        return "name";
    case SortBySize:
        return "size";
    case SortByDate:
        return "date";
    }
    return "";
}

static const char* order_str(DbSortOrder order)
{
    switch (order)
    {
    case SortAscending:
        return "asc";
    case SortDescending:
        return "desc";
    }
    return "";
}

void pkgi_save_config(const Config& config)
{
    char data[4096];
    int len = 0;
    if (!config.games_url.empty())
        len += pkgi_snprintf(
                data + len,
                sizeof(data) - len,
                "url_games %s\n",
                config.games_url.c_str());
    if (!config.dlcs_url.empty())
        len += pkgi_snprintf(
                data + len,
                sizeof(data) - len,
                "url_dlcs %s\n",
                config.dlcs_url.c_str());
    if (!config.psm_games_url.empty())
        len += pkgi_snprintf(
                data + len,
                sizeof(data) - len,
                "url_psm_games %s\n",
                config.psm_games_url.c_str());
    if (!config.psx_games_url.empty())
        len += pkgi_snprintf(
                data + len,
                sizeof(data) - len,
                "url_psx_games %s\n",
                config.psx_games_url.c_str());
    if (!config.psp_games_url.empty())
        len += pkgi_snprintf(
                data + len,
                sizeof(data) - len,
                "url_psp_games %s\n",
                config.psp_games_url.c_str());
    if (!config.comppack_url.empty())
        len += pkgi_snprintf(
                data + len,
                sizeof(data) - len,
                "url_comppack %s\n",
                config.comppack_url.c_str());
    if (!config.comppack_index_url.empty())
        len += pkgi_snprintf(
                data + len,
                sizeof(data) - len,
                "url_comppack_index %s\n",
                config.comppack_index_url.c_str());
    if (!config.install_psp_psx_location.empty())
        len += pkgi_snprintf(
                data + len,
                sizeof(data) - len,
                "install_psp_psx_location %s\n",
                config.install_psp_psx_location.c_str());
    if (config.psm_readme_disclaimer)
        len += pkgi_snprintf(
                data + len,
                sizeof(data) - len,
                "psm_disclaimer_yes_i_read_the_readme NoPsmDrm\n");
    len += pkgi_snprintf(
            data + len, sizeof(data) - len, "sort %s\n", sort_str(config.sort));
    len += pkgi_snprintf(
            data + len,
            sizeof(data) - len,
            "order %s\n",
            order_str(config.order));
    len += pkgi_snprintf(data + len, sizeof(data) - len, "filter ");
    const char* sep = "";
    if (config.filter & DbFilterRegionASA)
    {
        len += pkgi_snprintf(data + len, sizeof(data) - len, "%sASA", sep);
        sep = ",";
    }
    if (config.filter & DbFilterRegionEUR)
    {
        len += pkgi_snprintf(data + len, sizeof(data) - len, "%sEUR", sep);
        sep = ",";
    }
    if (config.filter & DbFilterRegionJPN)
    {
        len += pkgi_snprintf(data + len, sizeof(data) - len, "%sJPN", sep);
        sep = ",";
    }
    if (config.filter & DbFilterRegionUSA)
    {
        len += pkgi_snprintf(data + len, sizeof(data) - len, "%sUSA", sep);
        sep = ",";
    }
    len += pkgi_snprintf(data + len, sizeof(data) - len, "\n");

    if (config.no_version_check)
    {
        len += pkgi_snprintf(
                data + len, sizeof(data) - len, "no_version_check 1\n");
    }
    if (config.custom_config)
    {
        len += pkgi_snprintf(
                data + len, sizeof(data) - len, "custom_config 1\n");
    }
    if (config.install_psp_as_pbp)
    {
        len += pkgi_snprintf(
                data + len, sizeof(data) - len, "install_psp_as_pbp 1\n");
    }

    pkgi_save(
            fmt::format("{}/config.txt", pkgi_get_config_folder()), data, len);
}

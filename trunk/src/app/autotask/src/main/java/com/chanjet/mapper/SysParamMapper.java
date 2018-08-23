package com.chanjet.mapper;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Select;

@Mapper
public interface SysParamMapper {
    @Select("select  key_value FROM S_PARAM where KEY = 'QR_AUTO_QUERY_INTERVAL'")
    String findQRQueryInterval();
}

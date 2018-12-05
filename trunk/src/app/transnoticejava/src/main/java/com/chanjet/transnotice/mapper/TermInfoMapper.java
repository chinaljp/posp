package com.chanjet.transnotice.mapper;

import com.chanjet.transnotice.entity.TermInfo;
import org.apache.ibatis.annotations.*;

/**
 * @ClassName: TermInfoMapper
 * @Description:
 * @author: GuoJiaqing
 * @Date: Create at 2018/9/4 17:55
 * @Modified By:
 * @version: 1.0.0
 */
@Mapper
public interface TermInfoMapper {
    /**
     * @Title: findTermInfoByParam
     * @Description: 
     * @Author: GuoJiaqing
     * @Date: Create at 2018/9/4 17:56
     * @Param: [termId]
     * @return: com.chanjet.transnotice.entity.TermInfo
     */
    @Select("select sn,model from b_dev_term where term_id = #{termId,jdbcType=VARCHAR}")
    @Results({
            @Result(property="sTermSn",column="sn"),
            @Result(property="sTermModel",column="model")
    })
    TermInfo findTermInfoByParam(@Param("termId") String termId);
}

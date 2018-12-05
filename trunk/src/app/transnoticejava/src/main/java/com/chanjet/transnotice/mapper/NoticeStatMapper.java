package com.chanjet.transnotice.mapper;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Update;

@Mapper
public interface NoticeStatMapper {
    /**
     * @Title: UpdNoticeStatByParam
     * @Description: 通知推送成功，更新通知推送状态
     * @Author: GuoJiaqing
     * @Date: Create at 2018/9/3 9:47
     * @Param: [tab_name, rrn, trans_date]
     * @return: int
     */
    @Update("update  ${tab_name} set notice_status = '1',notice_count = 1 WHERE rrn = #{rrn,jdbcType=VARCHAR} and trans_date = #{trans_date,jdbcType=VARCHAR}")
    public int UpdNoticeStatByParam(@Param("tab_name") String tab_name, @Param("rrn") String rrn, @Param("trans_date") String trans_date);

    /**
     * @Title: UpdNoticeCntByParam
     * @Description: 通知推送失败，更新通知推送次数
     * @Author: GuoJiaqing
     * @Date: Create at 2018/9/3 9:47
     * @Param: [tab_name, rrn, trans_date]
     * @return: int
     */
    @Update("update ${tab_name} set notice_count = notice_count + 1 WHERE rrn = #{rrn,jdbcType=VARCHAR} and trans_date = #{trans_date,jdbcType=VARCHAR}")
    public int UpdNoticeCntByParam(@Param("tab_name") String tab_name, @Param("rrn") String rrn, @Param("trans_date") String trans_date);
}

package com.chanjet.mapper;

import com.chanjet.entity.D0DetailInfo;
import com.chanjet.entity.InlineDetailInfo;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Select;
import org.apache.ibatis.annotations.Update;

import java.util.List;

@Mapper
public interface D0JournalMapper {
    @Select("select id,amount,merch_id,p_fee,trans_date,cnt,busi_type from b_d0_withdraw_trans_detail where cal_flag = 'N' and rownum<=#{num}")
    List<D0DetailInfo> findJournalD0(@Param("num") Integer state);


    @Update("update b_d0_withdraw_trans_detail set CAL_FLAG='Y' where id = #{id}")
    public int updFeeFlag(@Param("id") String id);
}

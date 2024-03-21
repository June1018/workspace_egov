package main.service.impl;

import java.util.List;

import org.springframework.stereotype.Repository;
import egovframework.rte.psl.dataaccess.EgovAbstractDAO;
import main.service.MemberVO;

@Repository("memberDAO")
public class MemberDAO extends EgovAbstractDAO{
	
	public String insertMember(MemberVO vo) {
		return (String)insert("memberDAO.insertMember", vo);
	}

	public int selectMemberIdCheck(String userid) {
		return (int)select("memberDAO.selectMemberIdCheck", userid);
	}

	public List<?> selectPostList(String dong) {
		return list("memberDAO.selectPostList", dong);
	}

	public int selectMemberCount(MemberVO vo) {
		System.out.println("DAO USERID :"+vo.getUserid());
		System.out.println("DAO PASS   :"+vo.getPass());		
		return (int) select("memberDAO.selectMemberCount", vo);
	}

}

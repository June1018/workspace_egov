package main.service.impl;

import java.util.List;

import org.springframework.stereotype.Repository;
import egovframework.rte.psl.dataaccess.EgovAbstractDAO;
import main.service.BoardVO;

@Repository("boardDAO")
public class BoardDAO extends EgovAbstractDAO{

	public String insertNBoard(BoardVO vo) {
		return (String)insert("boardDAO.insertNBoard", vo);
	}

	public List<?> selectNBoardList(BoardVO vo) {
		return list("boardDAO.selectNBoardList", vo);
	}

	public int selectNBoardTotal(BoardVO vo) {
		return (int)select("boardDAO.selectNBoardTotal", vo);
	}

	public BoardVO selectNBoardDetail(int unq) {
		return (BoardVO)select("boardDAO.selectNBoardDetail", unq);
	}

	public int updateNBoardHits(int unq) {
		return (int)update("boardDAO.updateNBoardHits", unq);
	}

	public int updateNBoard(BoardVO vo) {
		return update("boardDAO.updateNBoard",vo);
	}

	public int selectNBoardPass(BoardVO vo) {
		return (int)select("boardDAO.selectNBoardPass",vo);
		
	}

	public int deleteNBoard(BoardVO vo) {
		System.out.println("Board DAO Unq  : " + vo.getUnq());
		System.out.println("Board DAO pass : " + vo.getPass());		
		return delete("boardDAO.deleteNBoard", vo);
	}
}

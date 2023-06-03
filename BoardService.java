package main.service;

import java.util.List;

public interface BoardService {

	/*
	 * 일반게시판 저장처리
	 */
	public String insertNBoard(BoardVO vo) throws Exception;
	/*
	 * 일반게시판 화면목록
	 */
	public List<?> selectNBoardList(BoardVO vo) throws Exception;
	/*
	 * Total 개수 get
	 */
	public int selectNBoardTotal(BoardVO vo) throws Exception;
	/*
	 * 상세화면 -- result값 BoardVO 넣어준다.
	 */
	public BoardVO selectNBoardDetail(int unq) throws Exception;
    /*
     * 조회수 증가
     */
	public int updateNBoardHits(int unq) throws Exception;
	/*
	 * update 처리
	 */
	public int updateNBoard(BoardVO vo) throws Exception;
	/*
	 * 암호확인
	 */
	public int selectNBoardPass(BoardVO vo) throws Exception;
	/*
	 * 삭제처리
	 */
	public int deleteNBoard(BoardVO vo) throws Exception;
}

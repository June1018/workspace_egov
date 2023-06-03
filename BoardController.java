package main.web;

import java.util.List;
import javax.annotation.Resource;
import org.springframework.stereotype.Controller;
import org.springframework.ui.ModelMap;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseBody;
import main.service.BoardService;
import main.service.BoardVO;

@Controller
public class BoardController {

	@Resource(name="boardService")
	private BoardService boardService;
	
	@RequestMapping("/boardWrite.do")
	public String boardWrite() {
		return "board/boardWrite";
	}
	
	@RequestMapping("/boardWriteSave.do")
	@ResponseBody
	public String insertNBoard(BoardVO vo) throws Exception {
		String result = boardService.insertNBoard(vo);
		String msg = "";
		if (result == null) {
			msg = "ok";
		}else {
			msg = "fail";
		}
		return msg;
	}
	
	@RequestMapping("/boardList.do")
	public String selectNBoardList(BoardVO vo, ModelMap model) throws Exception {
		int unit = 5;
		//총데이터 개수
		int total = boardService.selectNBoardTotal(vo);
		//total 페이지수
		int totalPage = (int)Math.ceil((double)total/unit);
		//현재 출력화는 페이지
		int viewPage = vo.getViewPage();
		if (viewPage > totalPage  || viewPage < 0){
			viewPage = 1;
		}
		
		int startIndex = (viewPage-1) * unit +1;
		int endIndex = startIndex + (unit -1) ;
		// total 34 (4Page)
		// 1(1Page) ->34 ~25, 2(2Page) ->24 ~ 15, 3(3Page) -> 14~5, 4(4Page)->4~1  
		/*int p1 = total -  0;
		int p2 = total - 10;
		int p3 = total - 20;
		int p4 = total - 30;*/
		
		int startRowNo = total - (viewPage-1) * unit;
				
		vo.setStartIndex(startIndex);
		vo.setEndIndex(endIndex);
		
		List<?> list = boardService.selectNBoardList(vo); 
		System.out.println("list : " + list);

		model.addAttribute("rowNumber" , startRowNo);
		model.addAttribute("total"     , total);
		model.addAttribute("totalPage" , totalPage);
		model.addAttribute("resultList", list);
		
		return "board/boardList";
	}
	
    @RequestMapping("/boardDetail.do")
    public String selectNBoardDetail(BoardVO vo, ModelMap model)throws Exception {
    	/*
    	 * 조회수 증가
    	 */
    	boardService.updateNBoardHits(vo.getUnq());
    	/*
    	 * 상세보기
    	 */
    	BoardVO boardVO = boardService.selectNBoardDetail(vo.getUnq());
    	
    	String content = boardVO.getContent();  // content 값을 가져와서 뉴라인 \n
    	boardVO.setContent( content.replace("\n", "<br>") );
    	
    	model.addAttribute("boardVO" , boardVO);
    	
    	return "board/boardDetail";
    }
    
    @RequestMapping("/boardModifyWrite.do")
    public String selectNBoardModifyWrite(BoardVO vo, ModelMap model)throws Exception {
    	BoardVO boardVO = boardService.selectNBoardDetail(vo.getUnq());
    	model.addAttribute("boardVO", boardVO);
    	
    	return "board/boardModifyWrite";
    }
    @RequestMapping("/boardModifySave.do")
	@ResponseBody  //ibatis 형태로 비동기 전송 결과값 전달
    public String updateNBoard(BoardVO vo) throws Exception {

    	int result = 0;
    	
    	int count = boardService.selectNBoardPass(vo); // int count = 1;  정상적인 경우 리턴값 vo 사용자 입력한 내용이 다 있음.
		if (count == 1) {
			result = boardService.updateNBoard(vo);    // int result = 1; 정상적인 경우 리턴값 vo 사용자 입력한 내용이 다 있음.
		}else {
			result = -1;
		}
    	
		return result+"";
    }
    
    @RequestMapping("/passWrite.do")
    public String passWrite(int unq, ModelMap model) {
;
    	model.addAttribute("unq", unq);
    	return "board/passWrite";
    }
    
    @RequestMapping("/boardDelete.do")
	@ResponseBody          //ajax 있을때 
    public String deleteBoard(BoardVO vo) throws Exception{
    	/*
    	 * 암호일치 검사 count =1 일치함  count = 0 일치하지 않음
    	 */
    	int result = 0;
    	System.out.println("deleteNBoard[UNQ] :"+vo.getUnq());
    	System.out.println("deleteNBoard[Pass]:"+vo.getPass());
    	int count = boardService.selectNBoardPass(vo);  // count = 1; 정상
    	
    	if(count == 1) {
    	   result = boardService.deleteNBoard(vo);     //result = 1; 정상 result = 0;
    	}else if (count == 0) {
           result = -1;
    	}
    	return result+"";
    }
}

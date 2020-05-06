#include <iostream>
#include <cassert>

/*-------1---------2---------3---------4---------5---------6---------7---------8---------9--------*/

class Context;
union EventParams;
enum EventId {
    E0,
    E1,
    E2,
    E3,
    E4,
    E5
};

union EventParams {

};

/*------------------------------------------- Common ---------------------------------------------*/
class Common {
public:
    class Statemachine;

    class TopState {
    private:
        typedef TopState TThisState;
        typedef TopState TSuperState;
    protected:
        TopState() {}
    public:
        static TopState* GetInstance() {
            static TopState singleInstance;
            return &singleInstance;
        }
        virtual void Entry(Context* pContext, Statemachine* pStm) {
            if (pStm->IsEnterable<TSuperState>()) {
            }
        }
        virtual void Exit(Context* pContext, Statemachine* pStm) {
            if (pStm->IsExitable<TThisState>()) {
            }
        }
        virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
            return false;
        }
    };

    class TopStateStart: public TopState {
    public:
        static TopState* GetInstance() {
            static TopStateStart singleInstance;
            return &singleInstance;
        }
    };

    class Statemachine {
    public:
        Statemachine* m_pParentStm;
        TopState*     m_pCurrentState;
        TopState*     m_pLCAState;
        TopState*     m_pTargetState;
        TopState*     m_pSourceState;
        TopState*     m_pPseudostate;
        bool          m_bIsExternTrans;
    public:
        Statemachine():
            m_pParentStm(nullptr),
            m_pCurrentState(TopState::GetInstance()),
            m_pLCAState(nullptr),
            m_pTargetState(nullptr),
            m_pSourceState(nullptr),
            m_pPseudostate(TopState::GetInstance()),
            m_bIsExternTrans(false)
        {
        }
        template<class TCompositeState = TopState>
        bool IsIn(TopState* leaf) {
            bool bResult = dynamic_cast<TCompositeState*>(leaf) != nullptr;
            return bResult;
        }
        template<class TSuperState = TopState>
        bool IsEnterable() {
            if ((!IsIn<TSuperState>(m_pLCAState) || TSuperState::GetInstance() == m_pLCAState) || m_pLCAState == nullptr) {
                return true;
            }
            return false;
        }
        template<class TThisState = TopState>
        bool IsExitable() {
            bool isThisLCA = IsIn<TThisState>(m_pSourceState) && IsIn<TThisState>(m_pTargetState);
            if (!isThisLCA || m_bIsExternTrans) { 
                m_bIsExternTrans &= !isThisLCA;
                return true;
            } else { 
                m_pLCAState = TThisState::GetInstance();
            }
            return false;
        }
        bool IsFinished() {
            return m_pCurrentState == TopState::GetInstance() && m_pCurrentState == m_pPseudostate;
        }
        void EndTrans(Context* pContext) {
            m_pCurrentState = m_pTargetState;
            m_bIsExternTrans = false;
            m_pCurrentState->Entry(pContext, this);
        }
        void BgnTrans(Context* pContext, TopState* pTargetState) {
            m_pTargetState = pTargetState;
            m_pPseudostate = pTargetState;
            m_pCurrentState->Exit(pContext, this);
        }
        void BgnTrans(Context* pContext, TopState* pTargetState, TopState* pPseudostate) {
            m_pTargetState = pTargetState;
            m_pPseudostate = pPseudostate;
            m_pCurrentState->Exit(pContext, this);
        }
        virtual bool DefaultTrans(Context* pContext) = 0;
        virtual bool EventProc(Context* pContext, EventId nEventId, EventParams* pParams) = 0;
        bool RunToCompletion(Context* pContext) {
            bool bResult;
            do {
                m_pSourceState = m_pCurrentState;
                m_pLCAState = nullptr;
                bResult = DefaultTrans(pContext);
            } while (bResult);
            return bResult;
        }
        bool Reset(Context* pContext, Statemachine* pParentStm, TopState* pEntryPoint) {
            if (pEntryPoint == nullptr) {
                m_pPseudostate = TopStateStart::GetInstance();
            }
            m_pParentStm = pParentStm;
            return RunToCompletion(pContext);
        }
        bool Abort(Context* pContext) {
            m_pSourceState = TopState::GetInstance();
            BgnTrans(pContext, TopState::GetInstance());
            EndTrans(pContext);
            return true;
        }
    };
};

#if 1
/*------------------------------------------ Specific --------------------------------------------*/
using Statemachine = Common::Statemachine;
using TopState = Common::TopState;
using TopStateStart = Common::TopStateStart;

class MainStm: public Statemachine {
    /*------------------------------------------- S1 ---------------------------------------------*/
    class S1: TopState {
    private:
        typedef S1       TThisState;
        typedef TopState TSuperState;
        const char*      name = "S1";
    public:
        static TopState* GetInstance() {
            static TThisState singleInstance;
            return &singleInstance;
        }
        virtual void Entry(Context* pContext, Statemachine* pStm) {
            if (pStm->IsEnterable<TSuperState>()) {
                /* substms' reset */
                TSuperState::GetInstance()->Entry(pContext, pStm);
                /* entry */
                std::cout << name << " entry\n";
            }
        }
        virtual void Exit(Context* pContext, Statemachine* pStm) {
            if (pStm->IsExitable<TThisState>()) {
                /* exit */
                std::cout << name << " exit\n";
                TSuperState::GetInstance()->Exit(pContext, pStm);
                /* substms' abort */
            }
        }
        virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
            bool bResult = false;
            pStm->m_pSourceState = TThisState::GetInstance();
            switch (nEventId) {
            case E1:
                if (std::cout << "g1\n") {
                    pStm->BgnTrans(pContext, S2::GetInstance(), S2Start::GetInstance());
                    pStm->EndTrans(pContext);
                    bResult = true;
                }
                break;
            default:
                break;
            }
            return bResult;
        }
    };

    /*------------------------------------------- S2 ---------------------------------------------*/
    class S2: public TopState {
    private:
        typedef S2       TThisState;
        typedef TopState TSuperState;
        const char*      name = "S2";
    public:
        static TopState* GetInstance() {
            static TThisState singleInstance;
            return &singleInstance;
        }
        virtual void Entry(Context* pContext, Statemachine* pStm) {
            if (pStm->IsEnterable<TSuperState>()) {
                /* substms' reset */
                /* entry */
                TSuperState::GetInstance()->Entry(pContext, pStm);
                std::cout << name << " entry\n";
            }
        }
        virtual void Exit(Context* pContext, Statemachine* pStm) {
            if (pStm->IsExitable<TThisState>()) {
                /* exit */
                std::cout << name << " exit\n";
                TSuperState::GetInstance()->Exit(pContext, pStm);
                /* substms' abort */
            }
        }
        virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
            bool bResult = false;
            pStm->m_pSourceState = TThisState::GetInstance();
            switch (nEventId) {
            default:
                break;
            }
            return bResult;
        }
    };

    class S2Start: public S2 {
    public:
        static TopState* GetInstance() {
            static S2Start singleInstance;
            return &singleInstance;
        }
    };

    class S3: public S2 {
    private:
        typedef S3       TThisState;
        typedef S2       TSuperState;
        const char*      name = "S3";
    public:
        static TopState* GetInstance() {
            static TThisState singleInstance;
            return &singleInstance;
        }
        virtual void Entry(Context* pContext, Statemachine* pStm) {
            if (pStm->IsEnterable<TSuperState>()) {
                /* substms' reset */
                /* entry */
                TSuperState::GetInstance()->Entry(pContext, pStm);
                std::cout << name << " entry\n";
            }
        }
        virtual void Exit(Context* pContext, Statemachine* pStm) {
            if (pStm->IsExitable<TThisState>()) {
                /* exit */
                std::cout << name << " exit\n";
                TSuperState::GetInstance()->Exit(pContext, pStm);
                /* substms' abort */
            }
        }
        virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
            bool bResult = false;
            pStm->m_pSourceState = TThisState::GetInstance();
            switch (nEventId) {
            case E0:
                if (std::cout << "g\n") {
                    pStm->BgnTrans(pContext, S4::GetInstance());
                    std::cout << "a\n";
                    pStm->EndTrans(pContext);
                    bResult = true;
                }
                break;
            default:
                break;
            }
            return bResult;
        }
    };

    class S4: public S2 {
    private:
        typedef S4       TThisState;
        typedef S2       TSuperState;
        const char*      name = "S4";
    public:
        static TopState* GetInstance() {
            static TThisState singleInstance;
            return &singleInstance;
        }
        virtual void Entry(Context* pContext, Statemachine* pStm) {
            if (pStm->IsEnterable<TSuperState>()) {
                /* substms' reset */
                /* entry */
                TSuperState::GetInstance()->Entry(pContext, pStm);
                std::cout << name << " entry\n";
            }
        }
        virtual void Exit(Context* pContext, Statemachine* pStm) {
            if (pStm->IsExitable<TThisState>()) {
                /* exit */
                std::cout << name << " exit\n";
                TSuperState::GetInstance()->Exit(pContext, pStm);
                /* substms' abort */
            }
        }
        virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
            bool bResult = false;
            pStm->m_pSourceState = TThisState::GetInstance();
            switch (nEventId) {
            case E0:
                if (std::cout << "g\n") {
                    pStm->BgnTrans(pContext, S2::GetInstance());
                    std::cout << "a\n";
                    pStm->EndTrans(pContext);
                    bResult = true;
                }
                break;
            default:
                break;
            }
            return bResult;
        }
    };

    /*------------------------------------------- S5 ---------------------------------------------*/
    class S5: public TopState {
    private:
        typedef S5       TThisState;
        typedef TopState TSuperState;
        const char*      name = "S5";
    public:
        static TopState* GetInstance() {
            static TThisState singleInstance;
            return &singleInstance;
        }
        virtual void Entry(Context* pContext, Statemachine* pStm) {
            if (pStm->IsEnterable<TSuperState>()) {
                /* substms' reset */
                /* entry */
                TSuperState::GetInstance()->Entry(pContext, pStm);
                std::cout << name << " entry\n";
            }
        }
        virtual void Exit(Context* pContext, Statemachine* pStm) {
            if (pStm->IsExitable<TThisState>()) {
                /* exit */
                std::cout << name << " exit\n";
                TSuperState::GetInstance()->Exit(pContext, pStm);
                /* substms' abort */
            }
        }
        virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
            bool bResult = false;
            pStm->m_pSourceState = TThisState::GetInstance();
            switch (nEventId) {
            default:
                break;
            }
            return bResult;
        }
    };

    /*-------------------------------------- DefaultTrans ----------------------------------------*/
    virtual bool DefaultTrans(Context* pContext) {
        bool bResult = false;
        /* substms' run-to-completion */
        if (m_pPseudostate == TopStateStart::GetInstance()) {
            BgnTrans(pContext, S1::GetInstance());
            std::cout << "top init\n";
            EndTrans(pContext);
            bResult = true;
        } else if (m_pPseudostate == S2::GetInstance()) {
            BgnTrans(pContext, S5::GetInstance());
            std::cout << "S2 finished\n";
            EndTrans(pContext);
            bResult = true;
        } else if (m_pPseudostate == S2Start::GetInstance()) {
            BgnTrans(pContext, S3::GetInstance());
            std::cout << "S2 init\n";
            EndTrans(pContext);
            bResult = true;
        } else {
            /* entry-points */
            /* final-points */
        }
        return bResult;
    }

public:
    virtual bool EventProc(Context* pContext, EventId nEventId, EventParams* pParams) {
        bool bResult = false;
        m_pLCAState = nullptr;
        /* substms' event-proc */
        bResult = m_pCurrentState->EventProc(pContext, this, nEventId, pParams);
        RunToCompletion(pContext);
        return bResult;
    }
};

class Context {
    MainStm mainStm;
public:
    bool Start() {
        mainStm.Abort(this);
        return mainStm.Reset(this, nullptr, nullptr);
    }
    bool EventProc(EventId nEventId, EventParams* pParams) {
        return mainStm.EventProc(this, nEventId, pParams);
    }
};

/*---------------------------------------------------------------------------------------------*/

int main() {
    std::cout << "Hello World!\n";
    Context ctxTest;
    ctxTest.Start();
    ctxTest.EventProc(E1, nullptr);
    ctxTest.EventProc(E0, nullptr);
    ctxTest.EventProc(E0, nullptr);
}
#endif


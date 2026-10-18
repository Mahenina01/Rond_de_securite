import { Navigate, Outlet, useLocation } from "react-router";
import tokenService from "../service/tokenService";

const ProtectedRoute = () => {
  const location = useLocation();
  const token = tokenService.getToken();

  if (!token) {
    return <Navigate to="/signin" replace state={{ from: location }} />;
  }

  return <Outlet />;
};

export default ProtectedRoute;

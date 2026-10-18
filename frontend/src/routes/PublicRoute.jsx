import { Navigate, Outlet } from "react-router";
import tokenService from "../service/tokenService";

const PublicRoute = () => {
  const isAuthenticated = tokenService.isTokenValid();

  if (isAuthenticated) {
    return <Navigate to="/dashboard" replace />;
  }

  return <Outlet />;
};

export default PublicRoute;
